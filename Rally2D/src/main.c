#include "raylib.h"

#include <stddef.h>

#include "game.h"
#include "types.h"

int main(void)
{
    // Window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_TITLE);
    SetTargetFPS(TARGET_FPS);

    // Initialize
    Game game = { 0 };
    GameInit(&game, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Loop
    while (!WindowShouldClose() && game.state != STATE_QUIT) {
        // Events
        GameHandleInput(&game);

        // Update
        if (game.state == STATE_PLAYING)
            GameUpdate(&game);

        // Render
        GameRender(&game);

        // Reset
        if (game.state == STATE_GAMEOVER && IsKeyPressed(KEY_SPACE))
            GameInit(&game, SCREEN_WIDTH, SCREEN_HEIGHT);
    }

    // Exit
    GameQuit(&game);
    CloseWindow();

    return 0;
}
