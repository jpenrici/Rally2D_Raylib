#include "raylib.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_TITLE "Connect Dots"
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define BOX_WIDTH 750
#define BOX_HEIGHT 550

typedef enum {
    STATE_PLAYING,
    STATE_PAUSE,
    STATE_GAMEOVER,
    STATE_QUIT
} GameState;

typedef struct {
    Vector2 center;
    int radius;
} CircleCollider;

typedef struct {
    bool marked;
    Vector2 center;
    CircleCollider collider;
} Shape;

typedef struct {
    Vector2 center;
} Cursor;

typedef struct {
    Shape* shapes;
    unsigned elements;
    bool closed;
} Graph;

typedef struct {
    unsigned int level;
    unsigned int timer;
    Cursor cursor;
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

static void Generate(Graph* graph, unsigned int vertices);
static void InfoShape(const Shape* shape);
static void InfoCircleCollider(const CircleCollider* cc);
static void InfoGraph(const Graph* graph);

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
    game->level = 3;
    game->timer = 0;

    Cursor* cursor = &game->cursor;
    cursor->center = (Vector2) { .x = SCREEN_WIDTH / 2.0f, .y = SCREEN_HEIGHT / 2.0f };

    Generate(&game->graph, game->level);
    InfoGraph(&game->graph);

    game->state = STATE_PLAYING;
}

static void GameHandleInput(Game* game)
{
    if (game == NULL)
        return;

    // Mouse
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mousePos = GetMousePosition();
        game->cursor.center = mousePos;
        // TraceLog(LOG_INFO, "[Game] Time: %d Cursor: (%f, %f)\n", game->timer, game->cursor.center.x, game->cursor.center.y);
    }

    // Keyboard
    if (IsKeyPressed(KEY_Q)) {
        game->state = STATE_QUIT;
    }

    if (IsKeyPressed(KEY_R)) {
        TraceLog(LOG_INFO, "Restart...");
        GameInit(game);
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
    DrawGraph(&game->graph);
    DrawCursor(&game->cursor);
    DrawHud(game);
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
    int x = (int)((SCREEN_WIDTH - BOX_WIDTH) / 2.0f);
    int y = (int)((SCREEN_HEIGHT - BOX_HEIGHT) / 2.0f);
    DrawRectangle(x, y, BOX_WIDTH, BOX_HEIGHT, GRAY);
}

static void DrawShape(const Shape* shape)
{
    if (shape == NULL)
        return;

    Color color = shape->marked ? RED : YELLOW;
    DrawCircle(shape->center.x, shape->center.y, 20, color);
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

    for (int i = 0; i < graph->elements; ++i) {
        DrawShape(&graph->shapes[i]);
    }
}

static void DrawHud(const Game* game)
{
    // Draw: Display
    // Draw: Time, Score and Help.
    DrawText("Game Running Test! Press ESC to exit.", 50, 20, 20, LIGHTGRAY);
}

static void DrawGameOver(void)
{
    // Draw: Game Over
}

static void Generate(Graph* graph, unsigned int vertices)
{
    if (graph == NULL)
        return;

    if (vertices < 1)
        return;

    graph->shapes = malloc(sizeof(Shape) * vertices);
    if (graph->shapes == NULL)
        return;

    graph->elements = vertices;
    for (int i = 0; i < vertices; ++i) {
        Vector2 center = (Vector2) {
            .x = GetRandomValue(0, SCREEN_WIDTH),
            .y = GetRandomValue(0, SCREEN_HEIGHT)
        };
        CircleCollider cc = { .center = center, .radius = 10 };
        graph->shapes[i] = (Shape) { .center = center, .collider = cc, .marked = false };
    }
}

static void InfoShape(const Shape* shape)
{
    if (shape == NULL)
        return;

    printf("[Shape: (%f, %f), %s] ", shape->center.x, shape->center.y,
        shape->marked ? "marked" : "free");
    InfoCircleCollider(&shape->collider);
}

static void InfoCircleCollider(const CircleCollider* cc)
{
    if (cc == NULL)
        return;

    printf("[Circle Collider: (%f, %f), %d]", cc->center.x, cc->center.y, cc->radius);
}

static void InfoGraph(const Graph* graph)
{
    if (graph == NULL)
        return;

    for (int i = 0; i < graph->elements; ++i) {
        printf("[%d]: ", i);
        InfoShape(&graph->shapes[i]);
        printf("\n");
    }
}
