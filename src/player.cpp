#include "player.hpp"
#include "game.hpp"
#include "general.hpp"
#include <math.h>
#include <cinttypes>
#include HTTP_INCLUDE
#include JSON_INCLUDE

#ifdef ENGINE_STORAGE_INCLUDE
#include ENGINE_STORAGE_INCLUDE
#endif

Player::Player(const char *user_name, const char *user_pass) : Entity(username, ENTITY_PLAYER, Vector(4, 20), Vector(1.0f, 2.0f), nullptr)
{
    sprite_3d = ENGINE_MEM_NEW Sprite3D();
    if (!sprite_3d)
    {
        ENGINE_LOG_INFO("[Player:Player] Failed to create Sprite3D for player\n");
        return;
    }
    sprite_3d_type = SPRITE_3D_CUSTOM;
    sprite_rotation = 0.0f;
    sprite_3d->setActive(true);
    sprite_3d->setWireframe(WIREFRAME_ENABLED);
    direction = Vector(1, 0);                                    // facing east initially
    plane = Vector(0, 0.66);                                     // camera plane perpendicular to direction
    is_player = true;                                            // Mark this entity as a player (so level doesn't delete it)
    end_position = Vector(4, 20);                                // Initialize end position
    start_position = Vector(4, 20);                              // Initialize start position
    snprintf(username, sizeof(this->username), "%s", user_name); // set username with bounds checking
    name = this->username;                                       // set name
    snprintf(password, sizeof(this->password), "%s", user_pass); // set password with bounds checking
    this->is_visible = false;                                    // dont show player's sprite
}

Player::~Player()
{
}

void Player::collision(Entity *other, Game *game)
{
}

void Player::render(Draw *canvas, Game *game)
{
    if (!canvas || !game || !game->current_level)
    {
        return;
    }
}

void Player::update(Game *game)
{
    if (!game || !game->is_active || !game->current_level)
    {
        return;
    }
}