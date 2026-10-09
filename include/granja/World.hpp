#pragma once
#include "Types.hpp"
#include <map>
#include <set>
namespace granja {
class World {
public:
    explicit World(std::uint32_t seed=20261009):seed_(seed) {}
    std::uint32_t seed() const { return seed_; }
    Terrain terrain(int x,int y) const;
    bool blocked(Vec p,float radius) const;
    Vec move(Vec from,Vec delta,float radius) const;
    void stream(Vec center);
    Chunk& ensure(ChunkKey key);
    const std::set<ChunkKey>& active() const { return active_; }
    std::map<ChunkKey,Chunk>& chunks() { return chunks_; }
    const std::map<ChunkKey,Chunk>& chunks() const { return chunks_; }
    std::map<ChunkKey,ChunkRecord> records() const;
    static constexpr Vec secretHome{-1050,1200};
    static constexpr Vec nest{160,160};
    static constexpr Vec bossHome{1250,-550};
    std::uint32_t hash(int x,int y,int salt=0) const;
private:
    std::uint32_t seed_;
    std::map<ChunkKey,Chunk> chunks_;
    std::set<ChunkKey> active_;
    std::map<ChunkKey,ChunkRecord> dormant_;
    static ChunkRecord capture(const Chunk& chunk);
    Chunk generate(ChunkKey key) const;
};
}
