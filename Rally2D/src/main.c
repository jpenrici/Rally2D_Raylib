#include "raylib.h"

#include "game.h"
#include "types.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WINDOW_TITLE "Rally 2D"
#define TARGET_FPS 60

int main(void)
{
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(TARGET_FPS);

    Game game = { 0 };
    GameInit(&game, WINDOW_WIDTH, WINDOW_HEIGHT);

    while (!WindowShouldClose() && game.state != STATE_QUIT) {
        GameHandleInput(&game);

        if (game.state == STATE_PLAYING)
            GameUpdate(&game);

        GameRender(&game);

        if (game.state == STATE_GAMEOVER && IsKeyPressed(KEY_SPACE))
            GameInit(&game, WINDOW_WIDTH, WINDOW_HEIGHT);
    }

    GameQuit(&game);
    CloseWindow();

    return 0;
}
