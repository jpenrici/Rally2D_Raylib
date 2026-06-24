#include "raylib.h"

#include <stddef.h>

#include "assets.h"
#include "background.h"
#include "collision.h"
#include "game.h"
#include "input.h"
#include "render.h"
#include "sprite.h"
#include "types.h"

void GameInit(Game* game, int screenW, int screenH)
{
    if (game == NULL)
        return;

    static bool assetsLoaded = false;
    if (!assetsLoaded) {
        LoadAllAssets(game, screenW, screenH);
        assetsLoaded = true;
    }

    // Player
    Car* car = &game->car;

    int carW = car->sheet.frames ? car->sheet.frames[0].width : PLAYER_WIDTH;
    int carH = car->sheet.frames ? car->sheet.frames[0].height : PLAYER_HEIGHT;

    car->pos.x = (float)(screenW / 2);
    car->pos.y = (float)(screenH - carH);
    car->velocity = 0.0f;
    car->step = 0.0f;
    car->energy = ENERGY_MAX;
    car->distance = 0;
    car->frame = 0;
    car->animTimer = 0;

    // Obstacle
    Obstacle* obs = &game->obstacle;
    int obsW = obs->sheet.frames ? obs->sheet.frames[0].width : OBSTACLE_WIDTH;
    int obsH = obs->sheet.frames ? obs->sheet.frames[0].height : OBSTACLE_HEIGHT;

    obs->pos.x = car->pos.x;
    obs->pos.y = -(float)(obsH / 2);
    obs->frame = 0;
    (void)obsW;

    // Background
    InitScroll(&game->background);

    // Arrow
    game->hud.arrowFrame = 0;

    // State
    game->state = STATE_PLAYING;
}

void GameHandleInput(Game* game)
{
    if (game == NULL)
        return;

    HandleInput(game);
}

void GameUpdate(Game* game)
{
    if (game == NULL)
        return;

    // Player
    Car* car = &game->car;
    Obstacle* obs = &game->obstacle;
    int scrW = GetScreenWidth();
    int scrH = GetScreenHeight();

    car->step = car->velocity / (float)PLAYER_MOVE_HORIZONTAL;

    if (car->velocity <= 0.0f) {
        car->frame = 4;
        car->animTimer = 0;
    }

    if (car->velocity > 0.0f) {
        AnimateSprite(&car->frame, &car->animTimer, 0, 3, 3);
    }

    // Background
    UpdateScroll(&game->background, car->step / 2.0f);

    // Obstacle
    obs->pos.y += car->step / 2.0f;

    if (ObstacleExited(obs, scrH)) {
        car->distance++;
        ResetObstacle(obs, scrW);
    }

    if (CheckCenterPoint(car, obs)) {
        ResolveCollision(car, obs, scrW);
    }

    if (car->energy <= 0) {
        game->state = STATE_GAMEOVER;
    }
}

void GameRender(const Game* game)
{
    if (game == NULL)
        return;

    RenderFrame(game, GetScreenWidth(), GetScreenHeight());
}

void GameQuit(Game* game)
{
    if (game == NULL)
        return;

    UnloadAllAssets(game);
}
