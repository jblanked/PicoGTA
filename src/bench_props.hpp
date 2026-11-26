#pragma once
#include "bench_layout.hpp"
#include "pico-game-engine/engine/entity.hpp"
#include "prop_colliders.hpp"

class Level;

// Level owns the entities; this pool owns one shared mesh, texture and collider set.
class BenchProps
{
  public:
    bool create(Level *level);
    void update(const Map &map, float x, float z, float reach);
    bool blocks(float x0, float z0, float x1, float z1, float radius) const;
    void release();

  private:
    Sprite3D *mesh = nullptr;
    PropColliders boxes;
    Entity *entities[BenchLayout::CAPACITY] = {};
    BenchLayout::Placement placements[BenchLayout::CAPACITY];
    int count = 0;
};
