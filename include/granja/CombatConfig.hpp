#pragma once
#include "Types.hpp"
#include <array>
namespace granja {
struct DropSettings {
    std::array<int,3> chestPercent{5,10,15}; // Chick, Fox, Snake
    std::array<int,4> rarityPercent{70,23,6,1}; // Common, Rare, Epic, Legendary
    int chance(Species species) const {return species==Species::Chick?chestPercent[0]:species==Species::Fox?chestPercent[1]:species==Species::Snake?chestPercent[2]:0;}
    bool valid() const {int sum=0;for(auto n:chestPercent)if(n<0||n>100)return false;for(auto n:rarityPercent){if(n<0||n>100)return false;sum+=n;}return sum==100;}
};
struct CombatConfig {
    static int level(int n) {return std::clamp(n,1,40);}
    static Species species(int n) {return n<10?Species::Chick:n<20?Species::Fox:Species::Snake;}
    static int band(int n) {return n<10?0:n<20?1:2;}
    static float health(int n) {n=level(n);return 45.f+n*9.f+(n>=10?20.f:0.f);}
    static float speed(Species species,int n) {n=level(n);return species==Species::Fox?140.f+n*1.5f:species==Species::Snake?95.f+n:80.f+n*2.f;}
};
}
