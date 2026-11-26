#include "scenery_layer.hpp"

SceneryLayer::SceneryLayer(const char *name, Vector anchor, Vector size, CallbackEntityDrawGame render,
                           const Entity *viewer, float depth)
    : Entity(name, ENTITY_3D_SPRITE, anchor, size, nullptr, nullptr, nullptr, CallbackEntityGame(),
             CallbackEntityGame(), CallbackEntityGame(), render, CallbackEntityEntityGame(), false,
             SPRITE_3D_NONE, 0),
      viewer(viewer), depth(depth)
{
}

void SceneryLayer::update(Game *)
{
    if (viewer)
        position = Vector(viewer->position.x + viewer->direction.x * depth,
                          viewer->position.y + viewer->direction.y * depth);
}
