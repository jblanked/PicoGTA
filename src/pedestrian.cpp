#include "pedestrian.hpp"
#include "game.hpp"

Pedestrian::Pedestrian(PicoGTA *city, int index, const char *personName, Vector position, uint16_t color)
    : Entity(personName, ENTITY_NPC, position, Vector(0.6f, 1.5f),
             nullptr, nullptr, nullptr, CallbackEntityGame(), CallbackEntityGame(),
             CallbackEntityGame(), CallbackEntityDrawGame(), CallbackEntityEntityGame(),
             false, SPRITE_3D_CUSTOM, color),
      city(city),
      index(index),
      leg(0),
      turnCount(0)
{
    direction.x = 0.0f;
    direction.z = 1.0f;
    speed = 0.02f;

    sprite_3d = ENGINE_MEM_NEW Sprite3D(false);
    if (!sprite_3d)
    {
        sprite_3d_type = SPRITE_3D_NONE;
        return;
    }

    const bool isWoman = (index % 2) == 0;
    const char *meshPath = isWoman
                               ? ASSETS_FOLDER "/woman.sprite3d"
                               : ASSETS_FOLDER "/man.sprite3d";
    if (!sprite_3d->fromPath(meshPath, false))
    {
        ENGINE_MEM_DELETE sprite_3d;
        sprite_3d = nullptr;
        sprite_3d_type = SPRITE_3D_NONE;
        return;
    }

    const char *uvPath = nullptr;
    const char *texturePath = nullptr;
    const int outfitIndex = isWoman ? (index / 2) % 3 : ((index - 1) / 2) % 4;
    switch (outfitIndex)
    {
    case 0:
        uvPath = isWoman
                     ? ASSETS_FOLDER "/woman-summer.uv"
                     : ASSETS_FOLDER "/man-original.uv";
        texturePath = isWoman
                          ? ASSETS_FOLDER "/woman-summer.rgb565"
                          : ASSETS_FOLDER "/man-original.rgb565";
        break;
    case 1:
        uvPath = isWoman
                     ? ASSETS_FOLDER "/woman-night.uv"
                     : ASSETS_FOLDER "/man-summer.uv";
        texturePath = isWoman
                          ? ASSETS_FOLDER "/woman-night.rgb565"
                          : ASSETS_FOLDER "/man-summer.rgb565";
        break;
    case 2:
        uvPath = isWoman
                     ? ASSETS_FOLDER "/woman-sport.uv"
                     : ASSETS_FOLDER "/man-night.uv";
        texturePath = isWoman
                          ? ASSETS_FOLDER "/woman-sport.rgb565"
                          : ASSETS_FOLDER "/man-night.rgb565";
        break;
    case 3:
        uvPath = ASSETS_FOLDER "/man-sport.uv";
        texturePath = ASSETS_FOLDER "/man-sport.rgb565";
        break;
    }

    if (!sprite_3d->fromUVPath(uvPath) ||
        !sprite_3d->fromTexturePath(texturePath, 128, 64))
    {
        ENGINE_MEM_DELETE sprite_3d;
        sprite_3d = nullptr;
        sprite_3d_type = SPRITE_3D_NONE;
        return;
    }

    sprite_3d->setPosition(position);
    sprite_3d->setDepthSortEnabled(true);
    sprite_3d->setWireframe(false);
    sprite_3d_type = SPRITE_3D_CUSTOM;
    aabb_dirty = true;
}

void Pedestrian::update(Game *game)
{
    (void)game;
    if (city)
        city->updatePerson(this);
}

void Pedestrian::setDirection(float x, float z)
{
    direction.x = x;
    direction.z = z;
}