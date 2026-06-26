#include "raylib.h"

#include <stddef.h>

#include "assets.h"
#include "sprite.h"

void LoadAllAssets(Game* game, int screenW, int screenH)
{
    if (game == NULL)
        return;

    // Background
    game->background.texture = LoadTexture(BG_PATH);
    game->background.loaded = (game->background.texture.id != 0);
    game->background.scrollOffset = 0.0f;

    // Game-over
    game->gameOverTexture = LoadTexture(GAMEOVER_PATH);
    game->gameOverLoaded = (game->gameOverTexture.id != 0);

    // Arrow
    PrepareSheet(&game->hud.arrow, ARROW_PATH, ARROW_ROWS, ARROW_COLS, ARROW_WIDTH, ARROW_HEIGHT);

    // Car
    PrepareSheet(&game->car.sheet, PLAYER_PATH, PLAYER_ROWS, PLAYER_COLS, PLAYER_WIDTH, PLAYER_HEIGHT);

    // Energy bar
    PrepareSheet(&game->hud.energy, ENERGY_PATH, ENERGY_ROWS, ENERGY_COLS, ENERGY_WIDTH, ENERGY_HEIGHT);

    // Obstacle
    PrepareSheet(&game->obstacle.sheet, OBSTACLE_PATH, OBSTACLE_ROWS, OBSTACLE_COLS, OBSTACLE_WIDTH, OBSTACLE_HEIGHT);

    // Odometer
    PrepareSheet(&game->hud.odometer, ODOMETER_PATH, ODOMETER_ROWS, ODOMETER_COLS, ODOMETER_WIDTH, ODOMETER_HEIGHT);

    // Speedometer
    PrepareSheet(&game->hud.speedometer, SPEEDOMETER_PATH, SPEEDOMETER_ROWS, SPEEDOMETER_COLS, SPEEDOMETER_WIDTH, SPEEDOMETER_HEIGHT);
}

void UnloadAllAssets(Game* game)
{
    if (game == NULL)
        return;

    // Standalone textures
    if (game->background.loaded)
        UnloadTexture(game->background.texture);
    if (game->gameOverLoaded)
        UnloadTexture(game->gameOverTexture);

    // Spritesheets
    UnloadSheet(&game->car.sheet);
    UnloadSheet(&game->obstacle.sheet);
    UnloadSheet(&game->hud.arrow);
    UnloadSheet(&game->hud.energy);
    UnloadSheet(&game->hud.odometer);
    UnloadSheet(&game->hud.speedometer);
}
