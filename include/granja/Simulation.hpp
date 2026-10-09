#pragma once
#include "World.hpp"
namespace granja {
class Simulation {
public:
    explicit Simulation(std::uint32_t seed=20261009):world(seed) { world.stream(player.pos); }
    Player player;
    World world;
    std::vector<Effect> effects;
    std::string message{"Explore os campos. E: conversar com Dona Cocó no ninho."};
    float messageTimer{7}, elapsed{};
    bool saveRequested{},victoryEvent{};
    void update(float dt,const Input& input);
    bool upgrade(int attribute);
    void addXp(int amount);
    void attack(Vec aim);
    void collect();
    void notify(std::string text);
    bool atNest() const { return distance(player.pos,World::nest)<110; }
private:
    void updateEnemies(float dt);
    void hurt(float damage,Vec source);
    void effect(Vec pos,int kind,std::string text={});
};
}
