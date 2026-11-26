#pragma once
#include "pico-game-engine/engine/entity.hpp"

// Keep callback-rendered backdrop layers behind world props for every heading.
// Their sort position is independent of the city meshes' fixed geometry anchor.
class SceneryLayer : public Entity {
    const Entity *viewer;
    float depth;
public:
    SceneryLayer(const char *name, Vector anchor, Vector size,
                 CallbackEntityDrawGame render, const Entity *viewer, float depth)
        : Entity(name,ENTITY_3D_SPRITE,anchor,size,nullptr,nullptr,nullptr,
                 CallbackEntityGame(),CallbackEntityGame(),CallbackEntityGame(),
                 render,CallbackEntityEntityGame(),false,SPRITE_3D_NONE,0),
          viewer(viewer), depth(depth) {}
    void update(Game *) override {
        if (viewer) position=Vector(viewer->position.x+viewer->direction.x*depth,
                                    viewer->position.y+viewer->direction.y*depth);
    }
};
