#include "granja/World.hpp"
namespace granja {
std::uint32_t World::hash(int x,int y,int salt) const {
    std::uint32_t n=std::uint32_t(x)*374761393u+std::uint32_t(y)*668265263u+seed_+std::uint32_t(salt)*2246822519u;
    n=(n^(n>>13))*1274126177u; return n^(n>>16);
}
Terrain World::terrain(int x,int y) const {
    Vec center{(x+.5f)*TileSize,(y+.5f)*TileSize};
    if(distance(center,nest)<260 || distance(center,bossHome)<230 || distance(center,secretHome)<170) return Terrain::Meadow;
    if(x==2 || y==2 || (x%29==0 && y%9!=0)) return Terrain::Dirt;
    float wave=std::sin(x*.17f)+std::cos(y*.19f)+.45f*std::sin((x+y)*.09f);
    if(wave>1.78f) return Terrain::Water;
    auto h=hash(x,y);
    if(h%100<8) return Terrain::Tree;
    if(h%100<11) return Terrain::Rock;
    return wave<-.8f ? Terrain::Meadow:Terrain::Grass;
}
bool World::blocked(Vec p,float radius) const {
    if(std::abs(p.x)+radius>999000 || std::abs(p.y)+radius>999000) return true;
    // Solid footprint of the homestead; the porch remains accessible.
    Vec nearest{std::clamp(p.x,99.f,221.f),std::clamp(p.y,-34.f,64.f)};
    if(distance(p,nearest)<radius) return true;
    for(float side:{-85.f,85.f}) if(distance(p,secretHome+Vec{side,-55})<radius+24) return true;
    int x0=int(std::floor((p.x-radius)/TileSize)),x1=int(std::floor((p.x+radius)/TileSize));
    int y0=int(std::floor((p.y-radius)/TileSize)),y1=int(std::floor((p.y+radius)/TileSize));
    for(int y=y0;y<=y1;++y) for(int x=x0;x<=x1;++x) {
        auto t=terrain(x,y);
        if(t==Terrain::Tree || t==Terrain::Rock) {
            float r=t==Terrain::Tree?23.f:21.f;
            if(distance(p,{(x+.5f)*TileSize,(y+.5f)*TileSize})<radius+r) return true;
        } else if(t==Terrain::Water) {
            Vec q{std::clamp(p.x,x*TileSize,(x+1)*TileSize),std::clamp(p.y,y*TileSize,(y+1)*TileSize)};
            if(distance(p,q)<radius) return true;
        }
    }
    return false;
}
Vec World::move(Vec from,Vec delta,float radius) const {
    int steps=std::max(1,int(std::ceil(length(delta)/8.f)));
    Vec d=delta*(1.f/steps);
    for(int i=0;i<steps;++i) {
        Vec next{from.x+d.x,from.y}; if(!blocked(next,radius)) from.x=next.x;
        next={from.x,from.y+d.y}; if(!blocked(next,radius)) from.y=next.y;
    }
    return from;
}
Chunk World::generate(ChunkKey key) const {
    Chunk c; c.key=key;
    int id=0;
    for(int y=0;y<16;++y) for(int x=0;x<16;++x) {
        int tx=key.x*16+x,ty=key.y*16+y; auto h=hash(tx,ty,7);
        Vec p{(tx+.5f)*TileSize,(ty+.5f)*TileSize};
        auto t=terrain(tx,ty);
        if(blocked(p,18) || t==Terrain::Water || t==Terrain::Tree || t==Terrain::Rock || distance(p,nest)<150 || distance(p,bossHome)<180) continue;
        if(h%100<=8) ++id; // Reserve legacy IDs, but eggs only spawn after an enemy defeat.
        else if(h%100==9 || h%100==10) c.loot.push_back({id++,p,LootKind::Food,false});
        else if(h%100==11) c.loot.push_back({id++,p,LootKind::Chest,false});
        else if(h%100<14 && distance(p,nest)>400) {
            int tier=1+std::min(3,int(distance(p,nest)/2200)); float hp=55.f+tier*20.f;
            c.enemies.push_back({id++,p,p,hp,hp,0,0,tier,false});
        }
    }
    if(key==ChunkKey{0,0}) {
        c.enemies.push_back({1100,{500,160},{500,160},75,75,0,0,1,false});
        c.enemies.push_back({1101,{700,160},{700,160},95,95,0,0,2,false});
        c.loot.push_back({1001,{420,160},LootKind::Food,false});
    }
    if(key==chunkAt(secretHome)) {
        c.loot.push_back({3000,secretHome,LootKind::Chest,false});

    }
    if(key==chunkAt(bossHome)) c.enemies.push_back({2000,bossHome,bossHome,480,480,0,0,5,true});
    for(auto& enemy:c.enemies) {
        enemy.species=enemy.boss?Species::Chicken:enemy.id==1100?Species::Snake:enemy.id==1101?Species::Fox:enemy.id%3==0?Species::Snake:enemy.id%3==1?Species::Fox:Species::Chicken;
        c.loot.push_back({4000+enemy.id*2,enemy.home,LootKind::Egg,false,enemy.id,false});
        if(enemy.tier>=2 || enemy.id%4==0)
            c.loot.push_back({4001+enemy.id*2,enemy.home+Vec{24,0},LootKind::GoldenEgg,false,enemy.id,false});
    }
    return c;
}
Chunk& World::ensure(ChunkKey key) {
    auto it=chunks_.find(key);
    if(it==chunks_.end()) {
        Chunk chunk=generate(key);
        if(auto saved=dormant_.find(key);saved!=dormant_.end()) {
            for(auto& l:chunk.loot) l.collected=std::find(saved->second.collected.begin(),saved->second.collected.end(),l.id)!=saved->second.collected.end();
            for(auto& e:chunk.enemies) for(const auto& record:saved->second.enemies) if(record.id==e.id) {
                e.pos=record.pos;e.hp=record.hp;e.rewarded=record.rewarded;e.brain=e.hp==0?Brain::Dead:Brain::Wander;
            }
            for(auto& loot:chunk.loot) if(loot.sourceEnemy>=0) {
                const auto enemy=std::find_if(chunk.enemies.begin(),chunk.enemies.end(),[&](const Enemy& e){return e.id==loot.sourceEnemy;});
                loot.spawned=enemy!=chunk.enemies.end() && enemy->rewarded;
                if(loot.spawned) loot.pos=enemy->pos+Vec{loot.kind==LootKind::GoldenEgg?24.f:0.f,0};
            }
            dormant_.erase(saved);
        }
        it=chunks_.emplace(key,std::move(chunk)).first;
    }
    return it->second;
}
void World::stream(Vec center) {
    ChunkKey key=chunkAt(center); active_.clear();
    for(int y=-1;y<=1;++y) for(int x=-1;x<=1;++x) {
        ChunkKey k{key.x+x,key.y+y}; active_.insert(k); ensure(k);
    }
    for(auto it=chunks_.begin();it!=chunks_.end();) {
        if(!active_.contains(it->first)) {dormant_[it->first]=capture(it->second);it=chunks_.erase(it);} else ++it;
    }
}
ChunkRecord World::capture(const Chunk& c) {
    ChunkRecord record;
    for(const auto& l:c.loot) if(l.collected) record.collected.push_back(l.id);
    for(const auto& e:c.enemies) record.enemies.push_back({e.id,e.pos,e.hp,e.rewarded});
    return record;
}
std::map<ChunkKey,ChunkRecord> World::records() const {
    auto result=dormant_;
    for(const auto& [k,c]:chunks_) result[k]=capture(c);
    return result;
}
}
