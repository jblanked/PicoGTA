#pragma once
#include "pico-game-engine/engine/entity.hpp"

class SceneryLayer : public Entity
{
  public:
    SceneryLayer(const char *name, Vector anchor, Vector size, CallbackEntityDrawGame render,
                 const Entity *viewer, float depth);
    void update(Game *game) override;

  private:
    const Entity *viewer;
    float depth;
};
