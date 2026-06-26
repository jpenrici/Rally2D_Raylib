#include "raylib.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define SCREEN_TITLE "Connect Dots"
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

typedef enum {
    STATE_PLAYING,
    STATE_PAUSE,
    STATE_GAMEOVER,
    STATE_QUIT
} GameState;

typedef struct {
    Vector2 a, b, c, d;
} ShapeCollider;

typedef struct {
    bool marked;
    Vector2 center;
    ShapeCollider collider;
} Shape;

typedef struct {
    Vector2 center;
} Cursor;

typedef struct {
    Shape* graph;
    bool closed;
} Graph;

typedef struct {
    unsigned int level;
    unsigned int score;
    unsigned int timer;
    Cursor cursor;
    Shape shape;
    Graph graph;
    GameState state;
} Game;

static void GameInit(Game* game);
static void GameHandleInput(Game* game);
static void GameUpdate(Game* game);
static void GameRender(const Game* game);
static void GamePlay(Game* game);
static void GameQuit(Game* game);

static void DrawBackground(void);
static void DrawShape(const Shape* shape);
static void DrawCursor(const Cursor* cursor);
static void DrawGraph(const Graph* graph);
static void DrawHud(const Game* game);
static void DrawGameOver(void);

static void generate(Graph* graph);

int main(void)
{
    // Window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_TITLE);
    SetTargetFPS(60);

    // Initialize Game
    Game game = { 0 };
    GameInit(&game);

    // Game Loop
    while (!WindowShouldClose() && game.state != STATE_QUIT) {
        // Events
        GameHandleInput(&game);

        // Update
        if (game.state == STATE_PLAYING)
            GameUpdate(&game); // call GamePlay() -> Status Game

        // Render
        BeginDrawing();
        {
            ClearBackground(RAYWHITE);
            GameRender(&game);
        }
        EndDrawing();

        // Status
        if (game.state == STATE_GAMEOVER && IsKeyPressed(KEY_SPACE))
            GameInit(&game);
    }

    // Exit
    GameQuit(&game);

    CloseWindow();

    return 0;
}

static void GameInit(Game* game)
{
    // Test
    game->level = 1;
    game->score = 0;
    game->timer = 0;

    Shape* shape = &game->shape;
    shape->marked = false;
    int x = 100, y = 100, r = 10; // Test
    shape->center = (Vector2) { .x = x, .y = y };
    shape->collider = (ShapeCollider) {
        (Vector2) { x - r, y }, (Vector2) { x + r, y },
        (Vector2) { x + r, y + r }, (Vector2) { x, y + r }
    };

    Cursor* cursor = &game->cursor;
    cursor->center = (Vector2) { .x = 150, .y = 150 };

    game->state = STATE_PLAYING;
}

static void GameHandleInput(Game* game)
{
    if (game == NULL)
        return;

    // Mouse

    // Keyboard
    if (IsKeyPressed(KEY_Q)) {
        game->state = STATE_QUIT;
    }
}

static void GameUpdate(Game* game)
{
    game->timer++;

    // GamePlay(&game); // Game Rules
}

static void GameRender(const Game* game)
{
    if (game == NULL)
        return;

    // Test
    DrawBackground();
    DrawShape(&game->shape);
    DrawCursor(&game->cursor);
    DrawGraph(&game->graph);
}

static void GamePlay(Game* game)
{
    // If the cursor clicks on the shape and it is not selected, select and connect it.
    // Check if any lines intersect; if not, proceed; otherwise, cancel the connection.
    //
}

static void GameQuit(Game* game)
{
    if (game == NULL)
        return;
}

static void DrawBackground(void)
{
    DrawText("Game Running Test! Press ESC to exit.", 50, 20, 20, LIGHTGRAY);
}

static void DrawShape(const Shape* shape)
{
    if (shape == NULL)
        return;

    DrawCircle(shape->center.x, shape->center.y, 20, RED);
}

static void DrawCursor(const Cursor* cursor)
{
    if (cursor == NULL)
        return;

    DrawCircle(cursor->center.x, cursor->center.y, 20, GREEN);
}

static void DrawGraph(const Graph* graph)
{

    if (graph == NULL)
        return;

    DrawLine(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, DARKGRAY);
    DrawLine(SCREEN_WIDTH, 0, 0, SCREEN_HEIGHT, DARKGRAY);
}

static void DrawHud(const Game* game)
{
    // Draw: Display
    // Draw: Time, Score and Help.
}

static void DrawGameOver(void)
{
    // Draw: Game Over
}
