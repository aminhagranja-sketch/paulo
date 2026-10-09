#include "granja/Save.hpp"
#include "granja/TouchControls.hpp"
#include <emscripten/emscripten.h>
#include <nlohmann/json.hpp>
#include <memory>
#include <fstream>
using namespace granja;
using json=nlohmann::json;
namespace {
std::unique_ptr<Simulation> game=std::make_unique<Simulation>();
TouchControls touch;
std::string output;
float cameraX=160,cameraY=160;
json state() {
    auto& p=game->player;
    return {{"x",p.pos.x},{"y",p.pos.y},{"fx",p.facing.x},{"fy",p.facing.y},{"hp",p.hp},{"maxHp",p.maxHp()},{"stamina",p.stamina},
    {"level",p.level},{"xp",p.xp},{"nextXp",p.nextLevelXp()},{"coins",p.coins},{"eggs",p.eggs},{"gold",p.goldenEggs},{"food",p.food},{"kills",p.kills},
    {"corn",p.corn},{"feathers",p.feathers},{"materials",p.materials},{"evolutionItems",p.evolutionItems},{"healthUp",p.healthUp},{"attackUp",p.attackUp},{"speedUp",p.speedUp},{"bossDefeated",p.bossDefeated},{"moving",p.moving},{"walkTime",p.walkTime},
    {"attack",p.attackVisual},{"dodge",p.dodgeTimer},{"invulnerable",p.invulnerable},{"atNest",game->atNest()},{"atShop",game->atShop()},{"ready",p.readyForBoss()}};
}
}
extern "C" {
EMSCRIPTEN_KEEPALIVE void game_new() {game=std::make_unique<Simulation>();touch.reset();cameraX=cameraY=160;}
EMSCRIPTEN_KEEPALIVE void game_resize(float w,float h) {touch.resize(w,h);}
EMSCRIPTEN_KEEPALIVE int game_pointer_down(int id,float x,float y) {return int(touch.down(id,{x,y}));}
EMSCRIPTEN_KEEPALIVE void game_pointer_move(int id,float x,float y) {touch.move(id,{x,y});}
EMSCRIPTEN_KEEPALIVE void game_pointer_up(int id) {touch.up(id);}
EMSCRIPTEN_KEEPALIVE void game_cancel_input() {touch.reset();}
EMSCRIPTEN_KEEPALIVE void game_tick(float dt,float mx,float my,int flags,float ax,float ay) {
    Input i=touch.sample();if(length(i.move)==0)i.move={mx,my};
    i.attack=i.attack||bool(flags&1);i.dodge=i.dodge||bool(flags&2);i.eat=i.eat||bool(flags&4);i.interact=i.interact||bool(flags&8);
    i.aim={ax,ay};game->update(dt,i);
    cameraX+=(game->player.pos.x-cameraX)*std::min(1.f,dt*7);cameraY+=(game->player.pos.y-cameraY)*std::min(1.f,dt*7);
}
EMSCRIPTEN_KEEPALIVE const char* game_input(float mx,float my,int flags,float ax,float ay) {
    Input i=touch.sample();if(length(i.move)==0)i.move={mx,my};
    int merged=flags|(i.attack?1:0)|(i.dodge?2:0)|(i.eat?4:0)|(i.interact?8:0);
    output=json{{"mx",i.move.x},{"my",i.move.y},{"flags",merged},{"ax",ax},{"ay",ay}}.dump();return output.c_str();
}
EMSCRIPTEN_KEEPALIVE void game_seed_rng(unsigned seed) {game->randomState=seed?seed:1;}
EMSCRIPTEN_KEEPALIVE int game_configure(const char* data) {
    try {auto j=json::parse(data);DropSettings settings;settings.chestPercent=j.at("chestPercent").get<std::array<int,3>>();settings.rarityPercent=j.at("rarityPercent").get<std::array<int,4>>();if(!settings.valid())return 0;game->drops=settings;return 1;}catch(...){return 0;}
}
EMSCRIPTEN_KEEPALIVE int game_upgrade(int attribute) {return game->upgrade(attribute);}
EMSCRIPTEN_KEEPALIVE int game_needs_save() {return game->saveRequested;}
EMSCRIPTEN_KEEPALIVE const char* game_snapshot(float width,float height) {
    game->syncProgression();
    json j={{"player",state()},{"time",game->elapsed},{"camera",{cameraX,cameraY}},{"message",game->messageTimer>0?game->message:""},{"loot",json::array()},{"enemies",json::array()},{"tiles",json::array()},{"effects",json::array()},{"chests",json::array()}};
    int x0=int(std::floor((cameraX-width*.5f-100)/64)),y0=int(std::floor((cameraY-height*.5f-130)/64));
    int cols=std::min(80,int(width/64)+5),rows=std::min(80,int(height/64)+6);
    for(int y=y0;y<y0+rows;++y)for(int x=x0;x<x0+cols;++x)j["tiles"].push_back({x,y,int(game->world.terrain(x,y)),game->world.hash(x,y)});
    for(auto k:game->world.active()) {
        auto& c=game->world.ensure(k);
        for(const auto& l:c.loot)if(!l.collected && l.spawned)j["loot"].push_back({l.pos.x,l.pos.y,int(l.kind)});
        for(const auto& e:c.enemies)if(e.hp>0 || e.timer>0)j["enemies"].push_back({e.pos.x,e.pos.y,e.aim.x,e.aim.y,e.hp,e.maxHp,e.boss,int(e.brain),e.tier,int(e.species),e.timer,e.id});
    }
    for(const auto& c:game->chests) j["chests"].push_back({c.id,c.pos.x,c.pos.y,c.rarity,int(c.state),c.timer,c.level});
    for(const auto& e:game->effects)j["effects"].push_back({e.pos.x,e.pos.y,e.life/e.total,e.kind,e.text});
    j["controls"]={{"stick",{touch.stick.x,touch.stick.y}},{"knob",{touch.knob.x,touch.knob.y}},{"attack",{touch.attackButton.x,touch.attackButton.y}},{"dodge",{touch.dodgeButton.x,touch.dodgeButton.y}},{"eat",{touch.eatButton.x,touch.eatButton.y}},{"interact",{touch.interactButton.x,touch.interactButton.y}},{"radius",touch.radius},{"attacking",touch.attacking()}};
    output=j.dump();return output.c_str();
}
EMSCRIPTEN_KEEPALIVE const char* game_save() {game->syncProgression();std::string error;if(!Save::write(*game,"/save.json",error)){output="";return output.c_str();}std::ifstream in("/save.json");output=std::string(std::istreambuf_iterator<char>(in),{});game->saveRequested=false;return output.c_str();}
EMSCRIPTEN_KEEPALIVE int game_load(const char* data) {std::ofstream("/save.json")<<data;std::string error;bool ok=Save::read(*game,"/save.json",error);if(ok){cameraX=game->player.pos.x;cameraY=game->player.pos.y;touch.reset();}return ok;}
}
