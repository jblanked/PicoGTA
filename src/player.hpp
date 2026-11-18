#pragma once
#include "config.hpp"
#include "pico-game-engine/engine/entity.hpp"
#include "pico-game-engine/engine/game.hpp"
#include "pico-game-engine/engine/level.hpp"
#include "pico-game-engine/engine/draw.hpp"
#include "general.hpp"
#include "loading.hpp"

class Player : public Entity
{
public:
    Player(const char *userName, const char *userPass);
    ~Player();
    //
    void collision(Entity *other, Game *game) override;
    void render(Draw *canvas, Game *game) override;
    void update(Game *game) override;

private:
    char username[64] = {0}; // username for login/registeration requests
    char password[64] = {0}; // password for login/registration requests (set in constructor)
};