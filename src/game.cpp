#include "game.hpp"

PicoGTA::PicoGTA(const char *username, const char *password)
{
    // pass username and password to player
    player = ENGINE_MEM_NEW Player(username, password);
    if (!player)
    {
        ENGINE_LOG_INFO("[PicoGTA:PicoGTA] Failed to create Player instance\n");
        return;
    }
    // player->setGhoulsGame(this);

    gameTime = ENGINE_MEM_NEW Time();
    if (!gameTime)
    {
        ENGINE_LOG_INFO("[PicoGTA:PicoGTA] Failed to create Time instance\n");
        ENGINE_MEM_DELETE player;
        player = nullptr;
        return;
    }

    // gameSound = ENGINE_MEM_NEW Sound();
    // if (!gameSound)
    // {
    //     ENGINE_LOG_INFO("[GhoulsGame:GhoulsGame] Failed to create Sound instance\n");
    //     ENGINE_MEM_DELETE gameTime;
    //     gameTime = nullptr;
    //     ENGINE_MEM_DELETE player;
    //     player = nullptr;
    //     return;
    // }
}
PicoGTA::~PicoGTA()
{
    this->endGame();
    if (engine)
    {
        engine->stop();
        ENGINE_MEM_DELETE engine;
        engine = nullptr;
    }
    if (player)
    {
        ENGINE_MEM_DELETE player;
        player = nullptr;
    }
    if (gameTime)
    {
        ENGINE_MEM_DELETE gameTime;
        gameTime = nullptr;
    }
    // if (gameSound)
    // {
    //     ENGINE_MEM_DELETE gameSound;
    //     gameSound = nullptr;
    // }
    if (draw)
    {
        ENGINE_MEM_DELETE draw;
        draw = nullptr;
    }
}

Game *PicoGTA::getGame() const
{
    if (engine)
    {
        return engine->getGame();
    }
    return nullptr;
}

void PicoGTA::endGame()
{
    shouldExit = true;
    isGameRunning = false;
}

bool PicoGTA::initDraw()
{
    if (!draw)
    {
        draw = ENGINE_MEM_NEW Draw();
        if (!draw)
        {
            ENGINE_LOG_INFO("[PicoGTA:initDraw] Failed to create Draw instance\n");
            return false;
        }
    }
    return true;
}

void PicoGTA::updateDraw()
{
    gameTime->tick();
    draw->fillScreen(0xFFFF);
    // player->drawCurrentView(draw);
    draw->swap();
}

void PicoGTA::updateInput(int key, bool held)
{
    (void)held; // not used for now

    this->lastInput = key;
}