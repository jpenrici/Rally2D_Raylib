/*
 * ConnectDots is a puzzle game where the player must connect the dots
 * without crossing the connecting lines, in the shortest time possible.
 * With each completion, the number of dots to be connected increases.
 *
 * The player can restart the entire game, pause, or quit.
 *
 * A simple puzzle game for practicing with raylib.
 *
 * >>> TO DO <<<
 */

#include "raylib.h"

#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_TITLE "Connect Dots"
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define PLAYING_AREA_WIDTH 750
#define PLAYING_AREA_HEIGHT 550
#define PLAYING_AREA_X (int)((SCREEN_WIDTH - PLAYING_AREA_WIDTH) / 2.0f)
#define PLAYING_AREA_Y (int)((SCREEN_HEIGHT - PLAYING_AREA_HEIGHT) / 2.0f)

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
static void GameLevel(Game* game);
static void GameHandleInput(Game* game);
static void GameUpdate(Game* game);
static void GameRender(const Game* game);
static void GamePlay(Game* game);
static void GameQuit(Game* game);

static void DrawBackground(void);
static void DrawShape(const Shape* shape);
static void DrawCursor(const Cursor* cursor);
static void DrawGraph(const Game* game);
static void DrawHud(const Game* game);
static void DrawGameOver(void);

static void SetGraph(Graph* graph, unsigned int vertices);

static bool isColliding(Vector2 v, const CircleCollider* cc);
static float distance(Vector2 v1, Vector2 v2);
static bool outside(Vector2 v);

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
            GameUpdate(&game);

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
    game->level = 0;
    game->timer = 0;

    GameLevel(game);

    game->state = STATE_PLAYING;
}

static void GameLevel(Game* game)
{
    game->level++;

    Cursor* cursor = &game->cursor;
    cursor->center = (Vector2) { .x = SCREEN_WIDTH / 2.0f, .y = SCREEN_HEIGHT / 2.0f };

    SetGraph(&game->graph, game->level);
    InfoGraph(&game->graph);
}

static void GameHandleInput(Game* game)
{
    if (game == NULL)
        return;

    // Mouse
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && game->state == STATE_PLAYING) {
        Vector2 mousePos = GetMousePosition();
        if (mousePos.x > PLAYING_AREA_X && mousePos.x < PLAYING_AREA_X + PLAYING_AREA_WIDTH && mousePos.y > PLAYING_AREA_Y && mousePos.y < PLAYING_AREA_Y + PLAYING_AREA_HEIGHT) {
            game->cursor.center = mousePos;
        }
    }

    // Keyboard
    if (IsKeyPressed(KEY_R)) {
        GameInit(game);
    }

    if (IsKeyPressed(KEY_P)) {
        if (game->state == STATE_PLAYING)
            game->state = STATE_PAUSE;
        else if (game->state == STATE_PAUSE)
            game->state = STATE_PLAYING;
    }

    if (IsKeyPressed(KEY_Q)) {
        game->state = STATE_QUIT;
    }
}

static void GameUpdate(Game* game)
{
    game->timer++;
    GamePlay(game);
}

static void GameRender(const Game* game)
{
    if (game == NULL)
        return;

    DrawBackground();
    DrawGraph(game);
    DrawCursor(&game->cursor);
    DrawHud(game);
}

static void GamePlay(Game* game)
{
    int counter = 0;
    for (int i = 0; i < game->graph.elements; ++i) {
        if (isColliding(game->cursor.center, &game->graph.shapes[i].collider)) {
            game->graph.shapes[i].marked = true;
        }
        if (game->graph.shapes[i].marked) {
            counter++;
        }
    }
    if (counter == game->graph.elements) {
        GameLevel(game);
    }
}

static void GameQuit(Game* game)
{
    if (game == NULL)
        return;
}

static void DrawBackground(void)
{
    DrawRectangle(
        PLAYING_AREA_X,
        PLAYING_AREA_Y,
        PLAYING_AREA_WIDTH,
        PLAYING_AREA_HEIGHT,
        GRAY);
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

static void DrawGraph(const Game* game)
{

    if (game == NULL)
        return;

    for (int i = 0; i < game->graph.elements; ++i) {
        Vector2 v1 = game->graph.shapes[i].center;
        Vector2 v2 = game->cursor.center;
        DrawLine(v1.x, v1.y, v2.x, v2.y, BLUE);
    }

    for (int i = 0; i < game->graph.elements; ++i) {
        DrawShape(&game->graph.shapes[i]);
    }
}

static void DrawHud(const Game* game)
{

    DrawText("Level: XX  Time: XX", 50, 5, 20, GRAY);
    if (game->state == STATE_PAUSE)
        DrawText("PAUSE", SCREEN_WIDTH - 100, 5, 20, RED);

    if (game->state == STATE_GAMEOVER) {
        DrawGameOver();
        return;
    }

    DrawText("Game Running Test! Press ESC to exit or R to restart", 50, SCREEN_HEIGHT - 20, 20, LIGHTGRAY);
}

static void DrawGameOver(void)
{
    DrawText("Game Over Test! Press ESC to exit or SPACE to restart", 50, SCREEN_HEIGHT - 20, 20, GREEN);
}

static void SetGraph(Graph* graph, unsigned int vertices)
{
    if (graph == NULL)
        return;

    if (vertices < 1)
        return;

    graph->shapes = malloc(sizeof(Shape) * vertices);
    if (graph->shapes == NULL)
        return;

    int radius = 10;
    graph->elements = vertices;
    for (int i = 0; i < vertices; ++i) {
        bool stop;
        do {
            stop = false;
            Vector2 center = (Vector2) {
                .x = PLAYING_AREA_X + GetRandomValue(PLAYING_AREA_X, PLAYING_AREA_WIDTH - PLAYING_AREA_X),
                .y = PLAYING_AREA_Y + GetRandomValue(PLAYING_AREA_Y, PLAYING_AREA_HEIGHT - PLAYING_AREA_Y)
            };
            CircleCollider cc = { .center = center, .radius = radius };
            graph->shapes[i] = (Shape) { .center = center, .collider = cc, .marked = false };
            for (int j = 0; j < i - 1; ++j) {
                if (isColliding(center, &graph->shapes[j].collider)) {
                    stop = true;
                    break;
                }
            }
        } while (stop);
    }
}

static bool isColliding(Vector2 v, const CircleCollider* cc)
{
    if (cc == NULL)
        return false;

    return distance(v, cc->center) <= cc->radius;
}

static float distance(Vector2 v1, Vector2 v2)
{
    float dx = v2.x - v1.x;
    float dy = v2.y - v1.y;
    return sqrtf(dx * dx + dy * dy);
}

static bool outside(Vector2 v)
{
    return v.x < PLAYING_AREA_X || v.x > PLAYING_AREA_X || v.y < PLAYING_AREA_Y || v.y > PLAYING_AREA_Y;
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
