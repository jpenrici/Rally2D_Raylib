/*
 * ConnectDots - puzzle game where the player connects dots with the cursor,
 * without crossing already drawn lines, in the shortest time possible.
 * Each completed level adds one more dot to the playing area.
 *
 * Controls:
 *   Mouse (click) — move the cursor
 *   R             — restart the game
 *   P             — pause / resume
 *   Q / ESC       — quit
 */

#include "raylib.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

#define SCREEN_TITLE "Connect Dots"
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define AREA_W 720
#define AREA_H 520
#define AREA_X ((SCREEN_WIDTH - AREA_W) / 2)
#define AREA_Y ((SCREEN_HEIGHT - AREA_H) / 2)

#define DOT_RADIUS 14 // dot visual radius
#define DOT_COLLIDER_R 18 // dot collision detection radius
#define DOT_MIN_DIST 60 // minimum distance between dot centers
#define CURSOR_RADIUS 10 // cursor visual radius
#define PATH_THICKNESS 3 // path line thickness

#define DOTS_START 3 // number of dots on level 1
#define TIMER_FPS 60 // target frames per second

// ---------------------------------------------------------------------------
// Types
// ---------------------------------------------------------------------------

typedef enum {
    STATE_PLAYING,
    STATE_PAUSE,
    STATE_WIN,
    STATE_GAMEOVER,
    STATE_QUIT
} GameState;

typedef struct {
    Vector2 center;
    float radius; // collider radius
    bool visited;
} Dot;

typedef struct {
    Vector2 center;
} Cursor;

typedef struct {
    Dot* dots;
    int count;
} Board;

// Sequence of visited dot centers used to draw the path lines
typedef struct {
    Vector2* points; // capacity = board.count
    int count; // number of points added so far
} Path;

typedef struct {
    int level;
    double levelStartTime; // GetTime() value at the start of the level
    double elapsed; // accumulated time before the last pause
    Cursor cursor;
    Board board;
    Path path;
    GameState state;
} Game;

// ---------------------------------------------------------------------------
// Prototypes
// ---------------------------------------------------------------------------

static void GameInit(Game* game);
static void GameStartLevel(Game* game);
static void GameFreeLevel(Game* game);
static void GameHandleInput(Game* game);
static void GameUpdate(Game* game);
static void GameRender(const Game* game);
static void GameCheckCollisions(Game* game);
static bool GameLevelComplete(const Game* game);
static void GameQuit(Game* game);

static void DrawBoard(const Game* game);
static void DrawDot(const Dot* dot);
static void DrawCursor(const Cursor* cursor);
static void DrawPath(const Path* path);
static void DrawHud(const Game* game);
static void DrawOverlay(const char* msg, Color bg, Color fg);

static void BoardInit(Board* board, int count);
static void PathInit(Path* path, int capacity);
static void PathAppend(Path* path, Vector2 point);

static bool CheckCircleCollision(Vector2 point, Vector2 center, float radius);
static bool PointInArea(Vector2 v);
static float Vec2Distance(Vector2 a, Vector2 b);
static bool SegmentsIntersect(Vector2 a, Vector2 b, Vector2 c, Vector2 d);
static bool PathCrossesNewSegment(const Path* path, Vector2 from, Vector2 to);

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------

int main(void)
{
    // Window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_TITLE);
    SetTargetFPS(TIMER_FPS);

    // Initialize Game
    Game game = { 0 };
    GameInit(&game);

    // Game Loop
    while (!WindowShouldClose() && game.state != STATE_QUIT) {
        // Keyboard and Mouse Events
        GameHandleInput(&game);

        // Update
        if (game.state == STATE_PLAYING)
            GameUpdate(&game);

        // Render
        BeginDrawing();
        {
            ClearBackground(DARKGRAY);
            GameRender(&game);
        }
        EndDrawing();
    }

    // Exit
    GameQuit(&game);
    CloseWindow();

    return 0;
}

// ---------------------------------------------------------------------------
// Game logic
// ---------------------------------------------------------------------------

static void GameInit(Game* game)
{
    GameFreeLevel(game);
    game->level = 0;
    game->elapsed = 0.0;
    game->state = STATE_PLAYING;
    GameStartLevel(game);
}

static void GameStartLevel(Game* game)
{
    GameFreeLevel(game);

    game->level++;
    game->elapsed = 0.0;
    game->levelStartTime = GetTime();

    // Cursor starts at the center of the playing area
    game->cursor.center = (Vector2) { AREA_X + AREA_W / 2.0f,
        AREA_Y + AREA_H / 2.0f };

    int dotCount = DOTS_START + (game->level - 1);
    BoardInit(&game->board, dotCount);
    PathInit(&game->path, dotCount);

    game->state = STATE_PLAYING;
}

static void GameFreeLevel(Game* game)
{
    // Board
    free(game->board.dots);
    game->board.dots = NULL;
    game->board.count = 0;

    // Path
    free(game->path.points);
    game->path.points = NULL;
    game->path.count = 0;
}

static void GameHandleInput(Game* game)
{
    // Pause / resume
    if (IsKeyPressed(KEY_P)) {
        if (game->state == STATE_PLAYING) {
            // Save elapsed time up to this point
            game->elapsed += GetTime() - game->levelStartTime;
            game->state = STATE_PAUSE;
        } else if (game->state == STATE_PAUSE) {
            // Resume: reset the time reference
            game->levelStartTime = GetTime();
            game->state = STATE_PLAYING;
        }
    }

    // Restart
    if (IsKeyPressed(KEY_R))
        GameInit(game);

    // Quit
    if (IsKeyPressed(KEY_Q))
        game->state = STATE_QUIT;

    // Restart after game over
    if (game->state == STATE_GAMEOVER && IsKeyPressed(KEY_SPACE))
        GameInit(game);

    // Move cursor by clicking inside the playing area
    if (game->state == STATE_PLAYING && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouse = GetMousePosition();
        if (PointInArea(mouse))
            game->cursor.center = mouse;
    }
}

static void GameUpdate(Game* game)
{
    GameCheckCollisions(game);

    if (GameLevelComplete(game)) {
        GameStartLevel(game);
    }
}

static void GameRender(const Game* game)
{
    // Playing area background
    DrawRectangle(AREA_X, AREA_Y, AREA_W, AREA_H, (Color) { 30, 30, 50, 255 });
    DrawRectangleLinesEx((Rectangle) { AREA_X, AREA_Y, AREA_W, AREA_H }, 2, LIGHTGRAY);

    DrawPath(&game->path);
    DrawBoard(game);
    DrawCursor(&game->cursor);
    DrawHud(game);

    if (game->state == STATE_PAUSE)
        DrawOverlay("PAUSED  [P to resume]", (Color) { 0, 0, 0, 160 }, RAYWHITE);

    if (game->state == STATE_GAMEOVER)
        DrawOverlay("GAME OVER  [SPACE to restart]", (Color) { 0, 0, 0, 200 }, RED);
}

static void GameCheckCollisions(Game* game)
{
    Board* board = &game->board;
    Path* path = &game->path;
    Cursor* cursor = &game->cursor;

    for (int i = 0; i < board->count; i++) {
        Dot* dot = &board->dots[i];

        if (dot->visited)
            continue;

        if (!CheckCircleCollision(cursor->center, dot->center, dot->radius))
            continue;

        // Check whether the line cursor → dot crosses any already drawn line
        if (path->count >= 2) {
            Vector2 from = path->points[path->count - 1];
            if (PathCrossesNewSegment(path, from, dot->center)) {
                // Crossing detected → game over
                game->state = STATE_GAMEOVER;
                return;
            }
        }

        dot->visited = true;
        PathAppend(path, dot->center);
        cursor->center = dot->center; // snap cursor to dot center
        break; // process one dot per frame to avoid double hits
    }
}

static bool GameLevelComplete(const Game* game)
{
    for (int i = 0; i < game->board.count; i++)
        if (!game->board.dots[i].visited)
            return false;
    return true;
}

static void GameQuit(Game* game)
{
    GameFreeLevel(game);
}

// ---------------------------------------------------------------------------
// Render helpers
// ---------------------------------------------------------------------------

static void DrawBoard(const Game* game)
{
    for (int i = 0; i < game->board.count; i++)
        DrawDot(&game->board.dots[i]);
}

static void DrawDot(const Dot* dot)
{
    Color fill = dot->visited ? (Color) { 80, 200, 120, 255 } : (Color) { 220, 180, 60, 255 };
    Color outline = dot->visited ? GREEN : YELLOW;
    DrawCircleV(dot->center, DOT_RADIUS, fill);
    DrawCircleV(dot->center, DOT_RADIUS - 3, ColorBrightness(fill, 0.3f));
    DrawCircleLinesV(dot->center, DOT_RADIUS, outline);
}

static void DrawCursor(const Cursor* cursor)
{
    DrawCircleV(cursor->center, CURSOR_RADIUS, (Color) { 100, 180, 255, 255 });
    DrawCircleLinesV(cursor->center, CURSOR_RADIUS, WHITE);
}

static void DrawPath(const Path* path)
{
    if (path->count < 2)
        return;

    for (int i = 0; i < path->count - 1; i++)
        DrawLineEx(path->points[i], path->points[i + 1], PATH_THICKNESS, (Color) { 100, 180, 255, 200 });
}

static void DrawHud(const Game* game)
{
    // Compute display time
    double displayTime = game->elapsed;
    if (game->state == STATE_PLAYING)
        displayTime += GetTime() - game->levelStartTime;

    char buf[64];
    snprintf(buf, sizeof(buf), "Level: %d   Time: %.1fs", game->level, displayTime);
    DrawText(buf, AREA_X + 4, AREA_Y - 28, 20, RAYWHITE);

    // Controls hint at the bottom
    DrawText("[R] Restart   [P] Pause   [Q] Quit", AREA_X + 4, AREA_Y + AREA_H + 6, 16, LIGHTGRAY);

    // Visited / total dot counter
    int visited = 0;
    for (int i = 0; i < game->board.count; i++)
        if (game->board.dots[i].visited)
            visited++;
    snprintf(buf, sizeof(buf), "%d / %d", visited, game->board.count);
    DrawText(buf, AREA_X + AREA_W - 60, AREA_Y - 28, 20, RAYWHITE);
}

static void DrawOverlay(const char* msg, Color bg, Color fg)
{
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, bg);
    int fontSize = 28;
    int w = MeasureText(msg, fontSize);
    DrawText(msg,
        (SCREEN_WIDTH - w) / 2,
        (SCREEN_HEIGHT - fontSize) / 2,
        fontSize, fg);
}

// ---------------------------------------------------------------------------
// Board & Path
// ---------------------------------------------------------------------------

static void BoardInit(Board* board, int count)
{
    board->dots = malloc(sizeof(Dot) * count);
    board->count = count;

    // Place dots at random positions, keeping a minimum distance between them
    for (int i = 0; i < count; i++) {
        Vector2 pos;
        bool tooClose;
        int attempts = 0;

        do {
            tooClose = false;
            pos = (Vector2) {
                (float)GetRandomValue(AREA_X + DOT_RADIUS + 4, AREA_X + AREA_W - DOT_RADIUS - 4),
                (float)GetRandomValue(AREA_Y + DOT_RADIUS + 4, AREA_Y + AREA_H - DOT_RADIUS - 4)
            };
            for (int j = 0; j < i; j++) {
                if (Vec2Distance(pos, board->dots[j].center) < DOT_MIN_DIST) {
                    tooClose = true;
                    break;
                }
            }
            attempts++;
        } while (tooClose && attempts < 500); // guard against infinite loop on very dense boards

        board->dots[i] = (Dot) {
            .center = pos,
            .radius = DOT_COLLIDER_R,
            .visited = false
        };
    }
}

static void PathInit(Path* path, int capacity)
{
    path->points = malloc(sizeof(Vector2) * capacity);
    path->count = 0;
}

static void PathAppend(Path* path, Vector2 point)
{
    path->points[path->count++] = point;
}

// ---------------------------------------------------------------------------
// Geometry / collision
// ---------------------------------------------------------------------------

static bool CheckCircleCollision(Vector2 point, Vector2 center, float radius)
{
    return Vec2Distance(point, center) <= radius;
}

static float Vec2Distance(Vector2 a, Vector2 b)
{
    return sqrt((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y));
}

static bool PointInArea(Vector2 v)
{
    return v.x >= AREA_X && v.x <= AREA_X + AREA_W && v.y >= AREA_Y && v.y <= AREA_Y + AREA_H;
}

/*
 * Returns the 2D cross product of vectors (o→a) and (o→b).
 * Used to determine the orientation of an ordered triplet of points.
 */
static float Cross2D(Vector2 o, Vector2 a, Vector2 b)
{
    return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

/*
 * Returns true if segment AB properly intersects segment CD.
 * Shared endpoints are intentionally excluded because consecutive path
 * segments always share a dot center.
 */
static bool SegmentsIntersect(Vector2 a, Vector2 b, Vector2 c, Vector2 d)
{
    float d1 = Cross2D(c, d, a);
    float d2 = Cross2D(c, d, b);
    float d3 = Cross2D(a, b, c);
    float d4 = Cross2D(a, b, d);

    if (((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) && ((d3 > 0 && d4 < 0) || (d3 < 0 && d4 > 0)))
        return true;

    return false;
}

/*
 * Returns true if the new segment (from → to) crosses any existing path
 * segment, skipping the immediately preceding one (which shares 'from').
 */
static bool PathCrossesNewSegment(const Path* path, Vector2 from, Vector2 to)
{
    for (int i = 0; i < path->count - 2; i++) {
        if (SegmentsIntersect(path->points[i], path->points[i + 1], from, to))
            return true;
    }
    return false;
}
