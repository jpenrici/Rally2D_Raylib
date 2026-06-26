#include "raylib.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

typedef enum {
    GAME_STATE_PLAYING,
    GAME_STATE_STOPED,
    GAME_STATE_EXIT
} GameState;

typedef struct {
    int width;
    int height;
} GameWindow;

typedef struct {
    bool isRunning;
    GameState state;
    float deltaTime;
    unsigned int score;
} Game;

void setup(Game* game);
void processInput(Game* game);
void update(Game* game);
void render(const Game* game);
bool checkStatus(Game* game);
void cleanUp(Game* game);
void quit(Game* game);

int main(void)
{
    Game game = { 0 };

    setup(&game);

    while (game.isRunning) {
        processInput(&game);
        update(&game);
        render(&game);
        if (checkStatus(&game)) {
        }
    }

    quit(&game); // call cleanUp(&Game)

    return 0;
}

void setup(Game* game) { puts("Setup..."); }
void processInput(Game* game) { }
void update(Game* game) { }
void render(const Game* game) { }
bool checkStatus(Game* game) { return true; }
void cleanUp(Game* game) { }
void quit(Game* game) { puts("Finished..."); }
