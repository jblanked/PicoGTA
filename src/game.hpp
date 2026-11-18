#pragma once
#include "config.hpp"
#include "pico-game-engine/engine/engine.hpp"
#include "general.hpp"
#include "player.hpp"
#include "time.hpp"
#include "sound.hpp"
#include "pedestrian.hpp"

class PicoGTA
{
public:
    PicoGTA(const char *username, const char *password);
    ~PicoGTA();

    void endGame();                                       // end the game and return to the submenu
    Time *getGameTime() const { return gameTime; }        // Get the game time instance
    int getCurrentInput() const { return lastInput; }     // Get the last input key pressed
    Draw *getDraw() const { return draw; }                // Get the Draw instance
    GameEngine *getEngine() const { return engine; }      // Get the game engine instance
    Game *getGame() const;                                // Get the Game instance
    Player *getPlayer() const { return player; }          // Get the player instance
    bool initDraw();                                      // Initialize the Draw instance (moved here for Flipper app; must call lcd_init_canvas first)
    bool isActive() const { return shouldExit == false; } // Check if the game is active
    bool isRunning() const { return isGameRunning; }      // Check if the game engine is running
    void resetInput() { lastInput = -1; }                 // Reset input after processing
    void updateDraw();                                    // update and draw the game
    void updateInput(int key, bool held);                 // update input for the game

private:
    bool dayJustSwitched = true;  // Flag to track if day/night just switched
    Draw *draw = nullptr;         // Draw instance
    GameEngine *engine = nullptr; // Engine instance
    bool isGameRunning = false;   // Flag to track if the game engine is running
    int lastInput = -1;           // Last input key pressed
    Player *player = nullptr;     // Player instance
    bool shouldExit = false;      // Flag to track if the game should exit
    Time *gameTime = nullptr;     // Game time instance
};