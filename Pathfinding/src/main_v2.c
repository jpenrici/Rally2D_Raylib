/*
 * A simple game to test the pathfinding algorithm.
 *
 * Controls:
 *   WASD or Arrow keys move the player
 *   P for pause
 *   R restarts after time runs out
 *   Q / ESC exits the game
 *
 * Build-time flags:
 *   -DONLY_SHAPE   hide image if loaded (development / CI)
 */

#include "raylib.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef ONLY_SHAPE
#define HIDDEN_IMAGE true
#else
#define HIDDEN_IMAGE false
#endif

// Window
#define SCREEN_TITLE "Game"
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 640
#define HUD_HEIGHT 40

// Grid
#define ROWS 15
#define COLS 20
#define CELL_HEIGHT ((SCREEN_HEIGHT - HUD_HEIGHT) / ROWS)
#define CELL_WIDTH (SCREEN_WIDTH / COLS)

// Spawn ranges
#define MIN_BONUS 5
#define MAX_BONUS 15
#define MIN_OBST 25
#define MAX_OBST 50

// Player (index 0) + Enemy (index 1) are fixed slots; the rest of the
// entity list is bonus/obstacle instances spawned at runtime.
#define PLAYER_INDEX 0
#define ENEMY_INDEX 1
#define MAX_ENTITIES (2 + MAX_BONUS + MAX_OBST)

#define FRAMES_COUNTER 20 // Throttle enemy movement speed

#define COLOR_BACKGROUND DARKBROWN
#define COLOR_GRID DARKGRAY

// If the sprite artwork's resting pose doesn't face "up", adjust this.
#define SPRITE_ROTATION_OFFSET 0.0f

// Structures
typedef struct {
    unsigned srcX, srcY; // offset inside the texture (pixels)
    unsigned srcW, srcH; // source rectangle size
    unsigned width, height; // display size (after resize)
} Sprite;

typedef struct {
    Texture2D texture;
    Sprite* frames; // [rows * cols] slices of the sheet
    unsigned count;
    bool loaded; // false -> draw the fallback shape instead
} SpriteSheet;

typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    NONE
} Direction;

typedef enum {
    KIND_PLAYER,
    KIND_ENEMY,
    KIND_BONUS,
    KIND_OBSTACLE,
    KIND_COUNT
} EntityKind;

typedef struct {
    int x, y;
} Point;

typedef struct {
    Point pos;
    Direction dir; // last movement direction, used to orient the sprite
    EntityKind kind;
    bool active; // false = removed (e.g. bonus already collected)
} Entity;

typedef enum {
    STATE_RUNNING,
    STATE_PAUSE,
    STATE_WIN,
    STATE_GAME_OVER,
    STATE_QUIT
} GameState;

typedef struct {
    Entity entities[MAX_ENTITIES];
    unsigned entityCount;

    bool walkable[COLS][ROWS]; // terrain only; entities are tracked separately
    SpriteSheet sheets[KIND_COUNT];

    GameState state;
    int hits;
    int score;
    unsigned total_bonus;
    unsigned frames_counter;
} Game;

// A* node
typedef struct Node {
    int x, y;
    bool walkable;
    int g_cost, h_cost, f_cost;
    struct Node* parent;
} Node;

// Per-kind visuals: sprite sheet source, fallback shape color, and whether
// it should rotate to face its movement direction.
typedef struct {
    const char* path;
    unsigned rows, cols, frameW, frameH;
    Color fallbackColor;
    bool rotates;
} EntityConfig;

static const EntityConfig kEntityConfig[KIND_COUNT] = {
    [KIND_PLAYER] = { "resources/player.png", 1, 2, 64, 64, BLUE, true },
    [KIND_ENEMY] = { "resources/enemy.png", 1, 2, 64, 64, GOLD, true },
    [KIND_BONUS] = { "resources/bonus.png", 1, 1, 64, 64, GREEN, false },
    [KIND_OBSTACLE] = { "resources/obstacle.png", 1, 1, 64, 64, RED, false },
};

// Single source of truth for direction -> movement delta and facing angle.
static const struct {
    int dx, dy;
    float angle;
} kDir[4] = {
    [UP] = { 0, -1, 0.0f },
    [DOWN] = { 0, 1, 180.0f },
    [LEFT] = { -1, 0, 270.0f },
    [RIGHT] = { 1, 0, 90.0f },
};

// Prototypes
static void GameInit(Game* game);
static void GameHandleInput(Game* game);
static void GameStep(Game* game); // moves the enemy, checks collisions/win
static void GameRender(const Game* game);

void LoadAllAssets(Game* game);
void UnloadAllAssets(Game* game);

static void PrepareSheet(SpriteSheet* sheet, const char* path, unsigned rows, unsigned cols,
    unsigned frameW, unsigned frameH);
static void ResizeSprite(SpriteSheet* sheet, unsigned newW, unsigned newH);
static void UnloadSheet(SpriteSheet* sheet);

static Rectangle CellRect(int x, int y);
static void DrawCell(int x, int y, Color c);
static void DrawSprite(const SpriteSheet* sheet, int gridX, int gridY, float rotation);
static void DrawEntityAt(const Game* game, const Entity* e);
static void DrawGRID(void);
static void DrawHUD(const Game* game);

static bool PointEq(Point a, Point b);
static Point PointAdd(Point p, int dx, int dy);
static bool IsInBounds(int x, int y);
static bool IsWalkable(const Game* game, int x, int y);
static bool IsOccupied(const Game* game, int x, int y);
static Point RandomEmptyCell(const Game* game);
static void SpawnEntity(Game* game, EntityKind kind);
static Direction Pathfinding(const Game* game, Point start, Point target);

// Main Entry Point
int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_TITLE);
    SetTargetFPS(60);

    Game game = { 0 };
    GameInit(&game);

    while (!WindowShouldClose() && game.state != STATE_QUIT) {
        GameHandleInput(&game);

        if (game.state == STATE_RUNNING)
            GameStep(&game);

        GameRender(&game);
    }

    UnloadAllAssets(&game);
    CloseWindow();

    return 0;
}

static void GameInit(Game* game)
{
    UnloadAllAssets(game);
    memset(game, 0, sizeof(*game));
    LoadAllAssets(game);

    game->state = STATE_RUNNING;

    for (int x = 0; x < COLS; x++)
        for (int y = 0; y < ROWS; y++)
            game->walkable[x][y] = true;

    // Reserve the fixed player/enemy slots before spawning bonus/obstacles,
    // so IsOccupied() correctly avoids their starting cells.
    game->entityCount = 2;

    game->entities[PLAYER_INDEX] = (Entity) {
        .pos = { GetRandomValue(0, COLS - 1), GetRandomValue(0, ROWS - 1) },
        .dir = UP,
        .kind = KIND_PLAYER,
        .active = true
    };

    do {
        game->entities[ENEMY_INDEX] = (Entity) {
            .pos = { GetRandomValue(0, COLS - 1), GetRandomValue(0, ROWS - 1) },
            .dir = UP,
            .kind = KIND_ENEMY,
            .active = true
        };
    } while (PointEq(game->entities[ENEMY_INDEX].pos, game->entities[PLAYER_INDEX].pos));

    game->total_bonus = (unsigned)GetRandomValue(MIN_BONUS, MAX_BONUS);
    for (unsigned i = 0; i < game->total_bonus; i++)
        SpawnEntity(game, KIND_BONUS);

    unsigned total_obstacles = (unsigned)GetRandomValue(MIN_OBST, MAX_OBST);
    for (unsigned i = 0; i < total_obstacles; i++)
        SpawnEntity(game, KIND_OBSTACLE);
}

static void GameHandleInput(Game* game)
{
    if (IsKeyPressed(KEY_P)) {
        if (game->state == STATE_RUNNING)
            game->state = STATE_PAUSE;
        else if (game->state == STATE_PAUSE)
            game->state = STATE_RUNNING;
    }

    if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_ESCAPE))
        game->state = STATE_QUIT;
    if (IsKeyPressed(KEY_R))
        GameInit(game);

    if (game->state != STATE_RUNNING)
        return;

    Direction moveDir = NONE;
    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP))
        moveDir = UP;
    else if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN))
        moveDir = DOWN;
    else if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT))
        moveDir = LEFT;
    else if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT))
        moveDir = RIGHT;

    if (moveDir == NONE)
        return;

    Entity* player = &game->entities[PLAYER_INDEX];
    player->dir = moveDir; // face the pressed direction even if the move is blocked

    Point next = PointAdd(player->pos, kDir[moveDir].dx, kDir[moveDir].dy);
    if (IsWalkable(game, next.x, next.y))
        player->pos = next;
}

static void GameStep(Game* game)
{
    game->frames_counter++;

    if (game->frames_counter % FRAMES_COUNTER == 0) {
        Entity* enemy = &game->entities[ENEMY_INDEX];
        Direction dir = Pathfinding(game, enemy->pos, game->entities[PLAYER_INDEX].pos);

        if (dir != NONE) {
            enemy->dir = dir;
            Point next = PointAdd(enemy->pos, kDir[dir].dx, kDir[dir].dy);
            if (IsWalkable(game, next.x, next.y))
                enemy->pos = next;
        }
    }

    Point playerPos = game->entities[PLAYER_INDEX].pos;

    if (PointEq(playerPos, game->entities[ENEMY_INDEX].pos)) {
        game->state = STATE_GAME_OVER;
        return;
    }

    for (unsigned i = 2; i < game->entityCount; i++) {
        Entity* e = &game->entities[i];
        if (e->kind == KIND_BONUS && e->active && PointEq(e->pos, playerPos)) {
            e->active = false;
            game->score += 10;
            game->hits++;
            break; // at most one bonus per cell
        }
    }

    if (game->hits >= (int)game->total_bonus)
        game->state = STATE_WIN;
}

static void GameRender(const Game* game)
{
    BeginDrawing();
    {
        ClearBackground(COLOR_BACKGROUND);
        DrawGRID();

        for (unsigned i = 2; i < game->entityCount; i++)
            if (game->entities[i].active)
                DrawEntityAt(game, &game->entities[i]);

        DrawEntityAt(game, &game->entities[PLAYER_INDEX]);
        DrawEntityAt(game, &game->entities[ENEMY_INDEX]);

        DrawHUD(game);
    }
    EndDrawing();
}

void LoadAllAssets(Game* game)
{
    for (unsigned k = 0; k < KIND_COUNT; k++) {
        const EntityConfig* cfg = &kEntityConfig[k];
        PrepareSheet(&game->sheets[k], cfg->path, cfg->rows, cfg->cols, cfg->frameW, cfg->frameH);
        ResizeSprite(&game->sheets[k], CELL_WIDTH, CELL_HEIGHT);
    }
}

void UnloadAllAssets(Game* game)
{
    for (unsigned k = 0; k < KIND_COUNT; k++)
        UnloadSheet(&game->sheets[k]);
}

static void PrepareSheet(SpriteSheet* sheet, const char* path, unsigned rows, unsigned cols,
    unsigned frameW, unsigned frameH)
{
    unsigned count = rows * cols;
    sheet->frames = (Sprite*)malloc(sizeof(Sprite) * (size_t)count);
    sheet->count = count;
    sheet->loaded = false;

    if (!sheet->frames) {
        fprintf(stderr, "[sprite] malloc failed for %s\n", path);
        return;
    }

    unsigned idx = 0;
    for (unsigned r = 0; r < rows; ++r) {
        for (unsigned c = 0; c < cols; ++c) {
            sheet->frames[idx++] = (Sprite) {
                .srcX = c * frameW, .srcY = r * frameH, .srcW = frameW, .srcH = frameH, .width = frameW, .height = frameH
            };
        }
    }

    sheet->texture = LoadTexture(path);
    sheet->loaded = (sheet->texture.id != 0);
    if (!sheet->loaded)
        fprintf(stderr, "[sprite] could not load: %s\n", path);
}

static void ResizeSprite(SpriteSheet* sheet, unsigned newW, unsigned newH)
{
    for (unsigned i = 0; i < sheet->count; ++i) {
        if (newW > 0)
            sheet->frames[i].width = newW;
        if (newH > 0)
            sheet->frames[i].height = newH;
    }
}

static void UnloadSheet(SpriteSheet* sheet)
{
    if (sheet->frames) {
        free(sheet->frames);
        sheet->frames = NULL;
    }

    sheet->count = 0;
    if (sheet->loaded) {
        UnloadTexture(sheet->texture);
        sheet->loaded = false;
    }
}

// Pixel rectangle of a grid cell, shared by DrawCell and DrawSprite so both
// always agree on where cell (x, y) actually is on screen.
static Rectangle CellRect(int x, int y)
{
    return (Rectangle) {
        .x = (float)(x * CELL_WIDTH),
        .y = (float)(HUD_HEIGHT + (y * CELL_HEIGHT)),
        .width = (float)CELL_WIDTH,
        .height = (float)CELL_HEIGHT
    };
}

static void DrawCell(int x, int y, Color c)
{
    Rectangle rec = CellRect(x, y);
    rec.width -= 2; // padding for a cleaner grid look
    rec.height -= 2;
    DrawRectangleRec(rec, c);
}

static void DrawSprite(const SpriteSheet* sheet, int gridX, int gridY, float rotation)
{
    if (!sheet->loaded || sheet->count == 0)
        return;

    const Sprite* s = &sheet->frames[0];
    Rectangle src = { (float)s->srcX, (float)s->srcY, (float)s->srcW, (float)s->srcH };

    Rectangle dst = CellRect(gridX, gridY);
    dst.width = (float)s->width;
    dst.height = (float)s->height;

    Vector2 origin = { dst.width / 2.0f, dst.height / 2.0f };
    dst.x += origin.x;
    dst.y += origin.y;

    DrawTexturePro(sheet->texture, src, dst, origin, rotation, WHITE);
}

static void DrawEntityAt(const Game* game, const Entity* e)
{
    const EntityConfig* cfg = &kEntityConfig[e->kind];
    const SpriteSheet* sheet = &game->sheets[e->kind];

    if (!sheet->loaded || HIDDEN_IMAGE) {
        DrawCell(e->pos.x, e->pos.y, cfg->fallbackColor);
        return;
    }

    float rotation = cfg->rotates ? kDir[e->dir].angle + SPRITE_ROTATION_OFFSET : 0.0f;
    DrawSprite(sheet, e->pos.x, e->pos.y, rotation);
}

static void DrawGRID(void)
{
    for (int i = 0; i <= COLS; i++)
        DrawLine(i * CELL_WIDTH, HUD_HEIGHT, i * CELL_WIDTH, SCREEN_HEIGHT, COLOR_GRID);
    for (int j = 0; j <= ROWS; j++)
        DrawLine(0, HUD_HEIGHT + (j * CELL_HEIGHT), SCREEN_WIDTH, HUD_HEIGHT + (j * CELL_HEIGHT), COLOR_GRID);
}

static void DrawHUD(const Game* game)
{
    DrawRectangle(0, 0, SCREEN_WIDTH, HUD_HEIGHT, BLACK);
    DrawLine(0, HUD_HEIGHT, SCREEN_WIDTH, HUD_HEIGHT, RAYWHITE);

    DrawText(TextFormat("STARS: %02d/%02d   SCORE: %04d", game->hits, game->total_bonus, game->score),
        20, 10, 20, RAYWHITE);

    if (game->state == STATE_PAUSE)
        DrawText("PAUSED (Press 'P' to Resume)", SCREEN_WIDTH - 320, 10, 20, GOLD);
    else if (game->state == STATE_WIN)
        DrawText("CONGRATULATIONS! Press 'R' to Restart", SCREEN_WIDTH - 400, 10, 20, RED);
    else if (game->state == STATE_GAME_OVER)
        DrawText("GAME OVER! Press 'R' to Restart", SCREEN_WIDTH - 360, 10, 20, RED);
    else
        DrawText("WASD/Arrows: Move | R: Restart | P: Pause", SCREEN_WIDTH - 360, 12, 16, LIGHTGRAY);
}

static bool PointEq(Point a, Point b) { return a.x == b.x && a.y == b.y; }
static Point PointAdd(Point p, int dx, int dy) { return (Point) { p.x + dx, p.y + dy }; }

static bool IsInBounds(int x, int y)
{
    return x >= 0 && x < COLS && y >= 0 && y < ROWS;
}

static bool IsWalkable(const Game* game, int x, int y)
{
    return IsInBounds(x, y) && game->walkable[x][y];
}

static bool IsOccupied(const Game* game, int x, int y)
{
    Point p = { x, y };
    for (unsigned i = 0; i < game->entityCount; i++) {
        if (game->entities[i].active && PointEq(game->entities[i].pos, p))
            return true;
    }
    return false;
}

static Point RandomEmptyCell(const Game* game)
{
    Point p;
    do {
        p.x = GetRandomValue(0, COLS - 1);
        p.y = GetRandomValue(0, ROWS - 1);
    } while (!game->walkable[p.x][p.y] || IsOccupied(game, p.x, p.y));
    return p;
}

static void SpawnEntity(Game* game, EntityKind kind)
{
    if (game->entityCount >= MAX_ENTITIES)
        return;

    Point p = RandomEmptyCell(game);
    game->entities[game->entityCount++] = (Entity) { .pos = p, .dir = UP, .kind = kind, .active = true };

    if (kind == KIND_OBSTACLE)
        game->walkable[p.x][p.y] = false;
}

// A* Pathfinding implementation targeting coordinates dynamically around solid blocks
static Direction Pathfinding(const Game* game, Point start, Point target)
{
    if (PointEq(start, target))
        return NONE;

    Node grid[COLS][ROWS];
    for (int x = 0; x < COLS; x++) {
        for (int y = 0; y < ROWS; y++) {
            grid[x][y].x = x;
            grid[x][y].y = y;
            grid[x][y].walkable = IsWalkable(game, x, y);
            grid[x][y].g_cost = 1000000; // Simulating Infinity
            grid[x][y].h_cost = abs(x - target.x) + abs(y - target.y); // Manhattan distance heuristic
            grid[x][y].f_cost = 1000000;
            grid[x][y].parent = NULL;
        }
    }

    bool open_set[COLS][ROWS] = { false };
    bool closed_set[COLS][ROWS] = { false };

    grid[start.x][start.y].g_cost = 0;
    grid[start.x][start.y].f_cost = grid[start.x][start.y].h_cost;
    open_set[start.x][start.y] = true;

    bool path_found = false;

    while (true) {
        int currentX = -1, currentY = -1;
        int min_f = 1000000;

        for (int x = 0; x < COLS; x++) {
            for (int y = 0; y < ROWS; y++) {
                if (open_set[x][y] && grid[x][y].f_cost < min_f) {
                    min_f = grid[x][y].f_cost;
                    currentX = x;
                    currentY = y;
                }
            }
        }

        if (currentX == -1)
            break; // Open set is empty, no path available

        if (currentX == target.x && currentY == target.y) {
            path_found = true;
            break;
        }

        open_set[currentX][currentY] = false;
        closed_set[currentX][currentY] = true;

        for (int i = 0; i < 4; i++) {
            int nx = currentX + kDir[i].dx;
            int ny = currentY + kDir[i].dy;

            if (nx >= 0 && nx < COLS && ny >= 0 && ny < ROWS) {
                if (!grid[nx][ny].walkable || closed_set[nx][ny])
                    continue;

                int tentative_g = grid[currentX][currentY].g_cost + 1;

                if (!open_set[nx][ny] || tentative_g < grid[nx][ny].g_cost) {
                    grid[nx][ny].parent = &grid[currentX][currentY];
                    grid[nx][ny].g_cost = tentative_g;
                    grid[nx][ny].f_cost = tentative_g + grid[nx][ny].h_cost;
                    open_set[nx][ny] = true;
                }
            }
        }
    }

    if (path_found) {
        Node* curr = &grid[target.x][target.y];
        while (curr->parent != NULL && (curr->parent->x != start.x || curr->parent->y != start.y)) {
            curr = curr->parent;
        }

        if (curr->x < start.x)
            return LEFT;
        if (curr->x > start.x)
            return RIGHT;
        if (curr->y < start.y)
            return UP;
        if (curr->y > start.y)
            return DOWN;
    }

    // Fallback: greedy straight-line movement if pathfinding is blocked
    if (start.x < target.x && IsWalkable(game, start.x + 1, start.y))
        return RIGHT;
    if (start.x > target.x && IsWalkable(game, start.x - 1, start.y))
        return LEFT;
    if (start.y < target.y && IsWalkable(game, start.x, start.y + 1))
        return DOWN;
    if (start.y > target.y && IsWalkable(game, start.x, start.y - 1))
        return UP;

    return NONE;
}
