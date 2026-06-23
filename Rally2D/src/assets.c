#include "raylib.h"

#include "assets.h"
#include "sprite.h"

void LoadAllAssets(Game* game, int screenW, int screenH)
{

    // Background
    game->background.texture = LoadTexture(ASSET_BACKGROUND);
    game->background.loaded = (game->background.texture.id != 0);
    game->background.scrollOffset = 0.0f;

    // Game-over
    game->gameOverTexture = LoadTexture(ASSET_GAME_OVER);
    game->gameOverLoaded = (game->gameOverTexture.id != 0);

    // Arrow
    PrepareSheet(&game->hud.arrow, ASSET_ARROW, 1, 5, 64, 64);

    // Car
    PrepareSheet(&game->car.sheet, ASSET_CAR, 1, 6, 128, 256);

    // Energy bar
    PrepareSheet(&game->hud.energy, ASSET_ENERGY, 1, 11, 48, 128);

    // Obstacle
    PrepareSheet(&game->obstacle.sheet, ASSET_OBSTACLE, 1, 6, 192, 192);

    // Odometer
    PrepareSheet(&game->hud.odometer, ASSET_ODOMETER, 51, 10, 34, 20);

    // Speedometer
    PrepareSheet(&game->hud.speedometer, ASSET_SPEEDOMETER, 7, 10, 128, 128);
}

void UnloadAllAssets(Game* game)
{
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
