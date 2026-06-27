#include "raylib.h"

#include <stddef.h>

#include "input.h"
#include "types.h"

void HandleInput(Game* game)
{
    if (IsKeyPressed(KEY_ESCAPE)) {
        game->state = STATE_QUIT;
        return;
    }

    if (game->state != STATE_PLAYING)
        return;

    Car* car = &game->car;
    int arrowFrame = 0;

    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
        arrowFrame = 1;
        if (car->velocity < VEL_MAX)
            car->velocity += 1.0f;
    }

    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
        arrowFrame = 2;
        car->frame = 5;
        if (car->velocity > VEL_MIN)
            car->velocity -= 1.0f;
    }

    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
        arrowFrame = 3;
        if (car->pos.x > BORDER_LEFT)
            car->pos.x -= car->step;
    }

    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
        arrowFrame = 4;
        if (car->pos.x < BORDER_RIGHT)
            car->pos.x += car->step;
    }

    game->hud.arrowFrame = arrowFrame;
}
