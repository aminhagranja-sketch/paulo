#include "granja/Save.hpp"
#include <nlohmann/json.hpp>
#include <cstdlib>
#include <fstream>
#include <set>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
namespace granja {
using json=nlohmann::json;
static json vectorJson(Vec p) { return json::array({p.x,p.y}); }
static Vec readVec(const json& j) {
    if(!j.is_array() || j.size()!=2) throw std::runtime_error("Coordenada inválida");
    Vec p{j.at(0).get<float>(),j.at(1).get<float>()};
    if(!std::isfinite(p.x) || !std::isfinite(p.y) || std::abs(p.x)>1000000 || std::abs(p.y)>1000000) throw std::runtime_error("Posição fora do mundo suportado");
    return p;
}
static int boundedInt(const json& j,const char* name,int min,int max) {
    int v=j.at(name).get<int>(); if(v<min || v>max) throw std::runtime_error(std::string("Valor inválido: ")+name); return v;
}
std::filesystem::path Save::defaultPath() {
#ifdef _WIN32
    if(const char* base=std::getenv("LOCALAPPDATA")) return std::filesystem::path(base)/"MeuGalinheiro"/"save.json";
#else
    if(const char* base=std::getenv("XDG_DATA_HOME")) return std::filesystem::path(base)/"meu-galinheiro"/"save.json";
    if(const char* base=std::getenv("HOME")) return std::filesystem::path(base)/".local"/"share"/"meu-galinheiro"/"save.json";
#endif
    return std::filesystem::current_path()/"saves"/"save.json";
}
bool Save::write(const Simulation& s,const std::filesystem::path& path,std::string& error) {
    try {
        const auto& p=s.player;
        json j={{"version",1},{"seed",s.world.seed()},{"elapsed",s.elapsed}};
        j["player"]={{"pos",vectorJson(p.pos)},{"facing",vectorJson(p.facing)},{"hp",p.hp},{"stamina",p.stamina},
            {"level",p.level},{"xp",p.xp},{"coins",p.coins},{"eggs",p.eggs},{"goldenEggs",p.goldenEggs},{"food",p.food},
            {"kills",p.kills},{"healthUp",p.healthUp},{"attackUp",p.attackUp},{"speedUp",p.speedUp},{"deaths",p.deaths},{"bossDefeated",p.bossDefeated}};
        j["chunks"]=json::array();
        for(const auto& [k,c]:s.world.records()) {
            json a={{"x",k.x},{"y",k.y},{"collected",json::array()},{"enemies",json::array()}};
            for(int id:c.collected) a["collected"].push_back(id);
            for(const auto& e:c.enemies) a["enemies"].push_back({{"id",e.id},{"pos",vectorJson(e.pos)},{"hp",e.hp},{"rewarded",e.rewarded}});
            j["chunks"].push_back(std::move(a));
        }
        if(!path.parent_path().empty()) std::filesystem::create_directories(path.parent_path());
        auto temporary=path; temporary+=".tmp";
        { std::ofstream stream(temporary,std::ios::binary|std::ios::trunc); stream.exceptions(std::ios::failbit|std::ios::badbit); stream<<j.dump(2); stream.flush(); stream.close(); }
#ifdef _WIN32
        if(!MoveFileExW(temporary.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)) throw std::runtime_error("Não foi possível substituir o save");
#else
        std::filesystem::rename(temporary,path);
#endif
        error.clear(); return true;
    } catch(const std::exception& e) { error=e.what(); return false; }
}
bool Save::read(Simulation& s,const std::filesystem::path& path,std::string& error) {
    try {
        if(std::filesystem::file_size(path)>32*1024*1024) throw std::runtime_error("Save excede 32 MB");
        std::ifstream stream(path,std::ios::binary); if(!stream) throw std::runtime_error("Save indisponível");
        json j; stream>>j;
        if(j.at("version").get<int>()!=1) throw std::runtime_error("Versão de save incompatível");
        Simulation candidate(j.at("seed").get<std::uint32_t>()); const auto& v=j.at("player"); auto& p=candidate.player;
        p.pos=readVec(v.at("pos")); p.facing=normalized(readVec(v.at("facing")));
        p.level=boundedInt(v,"level",1,10000); p.xp=boundedInt(v,"xp",0,p.nextLevelXp()-1);
        p.coins=boundedInt(v,"coins",0,100000000); p.eggs=boundedInt(v,"eggs",0,100000000);
        p.goldenEggs=boundedInt(v,"goldenEggs",0,100000000); p.food=boundedInt(v,"food",0,100000000);
        p.kills=boundedInt(v,"kills",0,100000000); p.deaths=boundedInt(v,"deaths",0,100000000);
        p.healthUp=boundedInt(v,"healthUp",0,5); p.attackUp=boundedInt(v,"attackUp",0,5); p.speedUp=boundedInt(v,"speedUp",0,5);
        p.hp=v.at("hp").get<float>(); p.stamina=v.at("stamina").get<float>(); p.bossDefeated=v.at("bossDefeated").get<bool>();
        if(!std::isfinite(p.hp) || p.hp<=0 || p.hp>p.maxHp() || !std::isfinite(p.stamina) || p.stamina<0 || p.stamina>100 || candidate.world.blocked(p.pos,PlayerRadius)) throw std::runtime_error("Estado do jogador inválido");
        candidate.elapsed=j.at("elapsed").get<float>();
        if(!std::isfinite(candidate.elapsed) || candidate.elapsed<0) throw std::runtime_error("Tempo inválido");
        const auto& chunks=j.at("chunks"); if(!chunks.is_array() || chunks.size()>20000) throw std::runtime_error("Lista de chunks inválida");
        std::set<ChunkKey> seen;
        for(const auto& a:chunks) {
            ChunkKey k{boundedInt(a,"x",-1000,1000),boundedInt(a,"y",-1000,1000)};
            if(!seen.insert(k).second) throw std::runtime_error("Chunk duplicado");
            auto& c=candidate.world.ensure(k);
            if(!a.at("collected").is_array() || !a.at("enemies").is_array()) throw std::runtime_error("Entidades inválidas");
            std::set<int> collected;
            for(const auto& id:a.at("collected")) collected.insert(id.get<int>());
            for(auto& l:c.loot) if(collected.erase(l.id)) l.collected=true;
            if(!collected.empty()) throw std::runtime_error("Item desconhecido");
            std::set<int> enemyIds;
            for(const auto& b:a.at("enemies")) {
                int id=b.at("id").get<int>();
                if(!enemyIds.insert(id).second) throw std::runtime_error("Inimigo duplicado");
                auto it=std::find_if(c.enemies.begin(),c.enemies.end(),[id](const Enemy& e){return e.id==id;});
                if(it==c.enemies.end()) throw std::runtime_error("Inimigo desconhecido");
                it->pos=readVec(b.at("pos")); it->hp=b.at("hp").get<float>(); it->rewarded=b.at("rewarded").get<bool>();
                if(!std::isfinite(it->hp) || it->hp<0 || it->hp>it->maxHp || it->rewarded!=(it->hp==0) || distance(it->pos,it->home)>1800 || candidate.world.blocked(it->pos,it->boss?27.f:18.f)) throw std::runtime_error("Estado de inimigo inválido");
                it->brain=it->hp==0?Brain::Dead:Brain::Wander;
            }
            if(enemyIds.size()!=c.enemies.size()) throw std::runtime_error("Inimigos ausentes");
        }
        candidate.world.stream(p.pos); candidate.notify("Progresso carregado. Boa aventura!");
        s=std::move(candidate); error.clear(); return true;
    } catch(const std::exception& e) { error=e.what(); return false; }
}
}
