#include "raylib.h"

#include "assets.h"
#include "background.h"
#include "collision.h"
#include "game.h"
#include "input.h"
#include "render.h"
#include "sprite.h"

void GameInit(Game* game, int screenW, int screenH)
{
    static bool assetsLoaded = false;
    if (!assetsLoaded) {
        LoadAllAssets(game, screenW, screenH);
        assetsLoaded = true;
    }

    Car* car = &game->car;

    int carW = car->sheet.frames ? car->sheet.frames[0].width : 128;
    int carH = car->sheet.frames ? car->sheet.frames[0].height : 256;

    car->pos.x = (float)(screenW / 2);
    car->pos.y = (float)(screenH - carH);
    car->velocity = 0.0f;
    car->step = 0.0f;
    car->energy = 10;
    car->distance = 0;
    car->frame = 4;
    car->animTimer = 0;

    Obstacle* obs = &game->obstacle;
    int obsW = obs->sheet.frames ? obs->sheet.frames[0].width : 192;
    int obsH = obs->sheet.frames ? obs->sheet.frames[0].height : 192;

    obs->pos.x = car->pos.x;
    obs->pos.y = -(float)(obsH / 2);
    obs->frame = 0;
    (void)obsW;

    InitScroll(&game->background);

    game->hud.arrowFrame = 0;
    game->state = STATE_PLAYING;
}

void GameHandleInput(Game* game)
{
    HandleInput(game);
}

void GameUpdate(Game* game)
{
    Car* car = &game->car;
    Obstacle* obs = &game->obstacle;
    int scrW = GetScreenWidth();
    int scrH = GetScreenHeight();

    car->step = car->velocity / 4.0f;

    if (car->velocity <= 0.0f) {
        car->frame = 4;
        car->animTimer = 0;
    }

    if (car->velocity > 0.0f) {
        AnimateSprite(&car->frame, &car->animTimer, 0, 3, 3);
    }

    UpdateScroll(&game->background, car->step / 2.0f);

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

    game->hud.arrowFrame = 0;
}

void GameRender(const Game* game)
{
    RenderFrame(game, GetScreenWidth(), GetScreenHeight());
}

void GameQuit(Game* game)
{
    UnloadAllAssets(game);
}
