#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>
#include <compare>
namespace granja {
struct Vec {
    float x{}, y{};
    Vec operator+(Vec b) const { return {x+b.x,y+b.y}; }
    Vec operator-(Vec b) const { return {x-b.x,y-b.y}; }
    Vec operator*(float k) const { return {x*k,y*k}; }
    Vec& operator+=(Vec b) { x+=b.x; y+=b.y; return *this; }
};
inline float length(Vec v) { return std::sqrt(v.x*v.x+v.y*v.y); }
inline Vec normalized(Vec v) { float l=length(v); return l>0.001f?v*(1.f/l):Vec{1,0}; }
inline float distance(Vec a,Vec b) { return length(a-b); }
inline float dot(Vec a,Vec b) { return a.x*b.x+a.y*b.y; }
inline constexpr float TileSize=64, ChunkSize=1024, PlayerRadius=18;
inline constexpr int TilesPerChunk=16;
struct ChunkKey {
    int x{},y{};
    auto operator<=>(const ChunkKey&) const = default;
};
inline ChunkKey chunkAt(Vec p) { return {int(std::floor(p.x/ChunkSize)),int(std::floor(p.y/ChunkSize))}; }
enum class Terrain { Grass, Meadow, Dirt, Water, Tree, Rock };
enum class LootKind { Egg, GoldenEgg, Food, Chest };
enum class Species { Chicken, Snake, Fox, Chick };
enum class Brain { Wander, Chase, Windup, Recover, Dead };
struct Loot { int id{}; Vec pos{}; LootKind kind{}; bool collected{}; int sourceEnemy{-1}; bool spawned{true}; };
struct Enemy {
    int id{}; Vec pos{},home{}; float hp{},maxHp{},timer{},cooldown{}; int tier{1}; bool boss{};
    Brain brain{Brain::Wander}; Vec aim{1,0}; bool rewarded{}; Species species{Species::Chicken};
};
struct EnemyRecord { int id{}; Vec pos{}; float hp{}; bool rewarded{}; };
struct ChunkRecord { std::vector<int> collected; std::vector<EnemyRecord> enemies; };
struct Chunk { ChunkKey key{}; std::vector<Loot> loot; std::vector<Enemy> enemies; };
enum class ChestState { Emerging, Closed, Opening, Open, Fading };
struct ChestDrop { std::uint32_t id{}; Vec pos{}; int rarity{},level{1}; ChestState state{ChestState::Emerging}; float timer{.35f}; bool rewarded{}; };
struct Player {
    Vec pos{160,160}, facing{1,0}; float hp{100},stamina{100},invulnerable{},attackCooldown{},attackVisual{},dodgeTimer{};
    Vec dodgeDirection{1,0}; float walkTime{}; bool moving{}; int level{1},xp{},coins{},eggs{},goldenEggs{},food{3},kills{},healthUp{},attackUp{},speedUp{},deaths{};
    int corn{},feathers{},materials{},evolutionItems{};
    bool bossDefeated{};
    float maxHp() const { return 100.f+(level-1)*12.f+healthUp*25.f; }
    float damage() const { return 24.f+(level-1)*4.f+attackUp*8.f; }
    float speed() const { return 205.f+speedUp*15.f; }
    int nextLevelXp() const { return 60+level*40; }
    bool readyForBoss() const { return eggs>=6 && goldenEggs>=2 && kills>=5; }
};
struct Input { Vec move{},aim{}; bool attack{},dodge{},eat{},interact{}; };
struct Effect { Vec pos{},velocity{}; float life{},total{}; int kind{}; std::string text; };
}
