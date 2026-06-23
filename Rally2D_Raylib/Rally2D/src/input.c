#include "raylib.h"

#include "input.h"

#define VEL_MAX 60.0f
#define VEL_MIN 0.0f

void HandleInput(Game* game)
{
    if (IsKeyPressed(KEY_ESCAPE)) {
        game->state = STATE_QUIT;
        return;
    }

    if (game->state == STATE_GAMEOVER) {
        return;
    }

    if (game->state != STATE_PLAYING)
        return;

    Car* car = &game->car;
    int arrowFrame = 0;

    if (IsKeyDown(KEY_W)) {
        arrowFrame = 1;
        if (car->velocity < VEL_MAX)
            car->velocity += 1.0f;
    }

    if (IsKeyDown(KEY_S)) {
        arrowFrame = 2;
        car->frame = 5;
        if (car->velocity > VEL_MIN)
            car->velocity -= 1.0f;
    }

    if (IsKeyDown(KEY_A)) {
        arrowFrame = 3;
        if (car->pos.x > 0.0f)
            car->pos.x -= car->step;
    }

    if (IsKeyDown(KEY_D)) {
        arrowFrame = 4;
        int screenW = GetScreenWidth();
        int carW = car->sheet.frames ? car->sheet.frames[0].width : 128;
        if (car->pos.x < (float)(screenW - carW))
            car->pos.x += car->step;
    }

    game->hud.arrowFrame = arrowFrame;
}
