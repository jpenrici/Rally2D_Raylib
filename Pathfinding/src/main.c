#include "raylib.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// Window
#define SCREEN_TITLE "Game"
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 640
#define HUD_HEIGHT 40

// Grid Configuration
#define ROWS 15
#define COLS 20
#define CELL_HEIGHT ((SCREEN_HEIGHT - HUD_HEIGHT) / ROWS)
#define CELL_WIDTH (SCREEN_WIDTH / COLS)

// Player
#define PLAYER_PATH "assets/player.png"
#define PLAYER_WIDTH 64
#define PLAYER_HEIGHT 64
#define PLAYER_ROWS 1
#define PLAYER_COLS 4

// Enemy
#define ENEMY_PATH "assets/enemy.png"
#define ENEMY_WIDTH 64
#define ENEMY_HEIGHT 64
#define ENEMY_ROWS 1
#define ENEMY_COLS 4

#define FRAMES_COUNTER 20 // Throttle Enemy movement speed

// Bonus
#define BONUS_PATH "assets/bonus.png"
#define BONUS_WIDTH 64
#define BONUS_HEIGHT 64
#define BONUS_ROWS 1
#define BONUS_COLS 1

#define MIN_BONUS 5
#define MAX_BONUS 15

// Obstacles
#define OBST_PATH "assets/obstacles.png"
#define OBST_WIDTH 64
#define OBST_HEIGHT 64
#define OBST_ROWS 1
#define OBST_COLS 3

#define MIN_OBST 25
#define MAX_OBST 50

// Colors
#define COLOR_PLAYER BLUE
#define COLOR_ENEMY GOLD
#define COLOR_BONUS GREEN
#define COLOR_OBSTACLE RED
#define COLOR_BACKGROUND DARKBROWN
#define COLOR_GRID DARKGRAY

// Structures
typedef struct {
    int srcX; // source X offset inside the texture (pixels)
    int srcY; // source Y offset inside the texture (pixels)
    int srcW; // source rectangle width
    int srcH; // source rectangle height
    int width; // image width
    int height; // image height
} Sprite;

typedef struct {
    Texture2D texture; // image with the Sprites
    Sprite* frames; // array of [rows * cols] Sprite values
    int count; // total number of frames (rows * cols)
    bool loaded; // if false, use fallback shape
} SpriteSheet;

typedef enum {
    CELL_EMPTY,
    CELL_BONUS,
    CELL_OBSTACLE
} CellKind;

typedef struct {
    SpriteSheet sheet;
    bool still; // non-animated image chosen from the available sprite options
    int animTimer; // counts frames before advancing the animation
    int frame; // current animation or image frame index
    CellKind cellKing;
} Entity;

typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    NONE
} Direction;

typedef struct {
    int x;
    int y;
} Point;

typedef enum {
    STATE_RUNNING,
    STATE_PAUSE,
    STATE_WIN,
    STATE_GAME_OVER,
    STATE_QUIT
} GameState;

typedef struct {
    Point player;
    Point enemy;

    CellKind map[COLS][ROWS];
    int total_bonus;
    int total_obstacles;

    Entity entity[4]; // 0 - Player, 1 - Enemy, 2 - Bonus and 3 - Obstacle
    GameState state;

    int hits;
    int score;
    int frames_counter; // Used to throttle enemy movement speed
} Game;

// A* Pathfinding Node Structure
typedef struct Node {
    int x, y;
    bool walkable;
    int g_cost; // Cost from start node
    int h_cost; // Heuristic cost to end node
    int f_cost; // g_cost + h_cost
    struct Node* parent;
} Node;

// Prototypes
static void GameInit(Game* game);
static void GameHandleInput(Game* game);
static void GameUpdate(Game* game);
static void GameRender(const Game* game);
static void GamePlay(Game* game);

void LoadAllAssets(Game* game);
void UnloadAllAssets(Game* game);

static void PrepareSheet(SpriteSheet* sheet, const char* path, unsigned rows, unsigned cols,
    unsigned frameW, unsigned frameH);

static void DrawSprite(const SpriteSheet* sheet, unsigned frame, float x, float y);
static void ResizeSprite(SpriteSheet* sheet, unsigned newW, unsigned newH);
static void UnloadSheet(SpriteSheet* sheet);

static void DrawCell(int x, int y, Color c);
static void DrawGRID(void);
static void DrawHUD(const Game* game);

static bool IsInBounds(int x, int y);
static bool IsWalkable(const Game* game, int x, int y);
static bool IsOccupied(const Game* game, int x, int y);
static Point RandomEmptyCell(const Game* game);
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

        if (game.state == STATE_RUNNING) {
            GameUpdate(&game);
            GamePlay(&game);
        }

        GameRender(&game);
    }

    CloseWindow();
    return 0;
}

static void GameInit(Game* game)
{
    game->state = STATE_RUNNING;

    game->hits = 0;
    game->score = 0;
    game->frames_counter = 0;

    // Clear the map
    for (int x = 0; x < COLS; x++) {
        for (int y = 0; y < ROWS; y++) {
            game->map[x][y] = CELL_EMPTY;
        }
    }

    // Random Player Position
    game->player.x = GetRandomValue(0, COLS - 1);
    game->player.y = GetRandomValue(0, ROWS - 1);

    // Random Enemy Position
    do {
        game->enemy.x = GetRandomValue(0, COLS - 1);
        game->enemy.y = GetRandomValue(0, ROWS - 1);
    } while (game->enemy.x == game->player.x && game->enemy.y == game->player.y);

    // Place Bonus
    game->total_bonus = GetRandomValue(MIN_BONUS, MAX_BONUS);
    for (int i = 0; i < game->total_bonus; i++) {
        Point p = RandomEmptyCell(game);
        game->map[p.x][p.y] = CELL_BONUS;
    }

    // Place Obstacles
    game->total_obstacles = GetRandomValue(MIN_OBST, MAX_OBST);
    for (int i = 0; i < game->total_obstacles; i++) {
        Point p = RandomEmptyCell(game);
        game->map[p.x][p.y] = CELL_OBSTACLE;
    }
}

static void GameHandleInput(Game* game)
{
    // State management keys
    if (IsKeyPressed(KEY_P)) {
        if (game->state == STATE_RUNNING)
            game->state = STATE_PAUSE;
        else if (game->state == STATE_PAUSE)
            game->state = STATE_RUNNING;
    }

    if (IsKeyPressed(KEY_Q)) {
        game->state = STATE_QUIT;
    }
    if (IsKeyPressed(KEY_R))
        GameInit(game);

    if (game->state != STATE_RUNNING)
        return;

    // Grid-based Player Movement Input.
    Point next = game->player;

    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
        next.y--;
    else if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
        next.y++;
    else if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
        next.x--;
    else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
        next.x++;

    if (IsWalkable(game, next.x, next.y)) {
        game->player = next;
    }
}

static void GameUpdate(Game* game)
{
    game->frames_counter++;

    if (game->frames_counter % FRAMES_COUNTER == 0) {
        Direction dir = Pathfinding(game, game->enemy, game->player);

        Point next_enemy = game->enemy;
        switch (dir) {
        case UP:
            next_enemy.y--;
            break;
        case DOWN:
            next_enemy.y++;
            break;
        case LEFT:
            next_enemy.x--;
            break;
        case RIGHT:
            next_enemy.x++;
            break;
        default:
            break; // No path found or reached target
        }

        if (IsWalkable(game, next_enemy.x, next_enemy.y)) {
            game->enemy = next_enemy;
        }
    }
}

static void GamePlay(Game* game)
{
    // Collision with Enemy (Game Over)
    if (game->player.x == game->enemy.x && game->player.y == game->enemy.y) {
        game->state = STATE_GAME_OVER;
        return;
    }

    CellKind* cell = &game->map[game->player.x][game->player.y];
    if (*cell == CELL_BONUS) {
        *cell = CELL_EMPTY;
        game->score += 10;
        game->hits++;
    }

    if (game->hits >= game->total_bonus) {
        game->state = STATE_WIN;
    }
}

static void GameRender(const Game* game)
{
    BeginDrawing();
    {
        ClearBackground(COLOR_BACKGROUND);

        DrawGRID();

        for (int x = 0; x < COLS; x++) {
            for (int y = 0; y < ROWS; y++) {
                if (game->map[x][y] == CELL_BONUS) {
                    DrawCell(x, y, COLOR_BONUS);
                } else if (game->map[x][y] == CELL_OBSTACLE) {
                    DrawCell(x, y, COLOR_OBSTACLE);
                }
            }
        }

        DrawCell(game->player.x, game->player.y, COLOR_PLAYER);
        DrawCell(game->enemy.x, game->enemy.y, COLOR_ENEMY);

        DrawHUD(game);
    }
    EndDrawing();
}

void LoadAllAssets(Game* game)
{
    // Player
    PrepareSheet(&game->entity[0].sheet, PLAYER_PATH, PLAYER_ROWS, PLAYER_COLS, PLAYER_WIDTH, PLAYER_WIDTH);
    ResizeSprite(&game->entity[0].sheet, CELL_WIDTH, CELL_HEIGHT);

    // Enemy
    PrepareSheet(&game->entity[1].sheet, ENEMY_PATH, ENEMY_ROWS, ENEMY_COLS, ENEMY_WIDTH, ENEMY_HEIGHT);
    ResizeSprite(&game->entity[1].sheet, CELL_WIDTH, CELL_HEIGHT);

    // Bonus
    PrepareSheet(&game->entity[2].sheet, BONUS_PATH, BONUS_ROWS, BONUS_COLS, BONUS_WIDTH, BONUS_HEIGHT);
    ResizeSprite(&game->entity[2].sheet, CELL_WIDTH, CELL_HEIGHT);

    // Obstacle
    PrepareSheet(&game->entity[3].sheet, OBST_PATH, OBST_ROWS, OBST_COLS, OBST_WIDTH, OBST_HEIGHT);
    ResizeSprite(&game->entity[3].sheet, CELL_WIDTH, CELL_HEIGHT);
}

void UnloadAllAssets(Game* game)
{
    for (unsigned i = 0; i < 4; ++i) {
        UnloadSheet(&game->entity[i].sheet);
    }
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
            Sprite* s = &sheet->frames[idx++];
            s->srcX = c * frameW;
            s->srcY = r * frameH;
            s->srcW = frameW;
            s->srcH = frameH;
            s->width = frameW;
            s->height = frameH;
        }
    }

    sheet->texture = LoadTexture(path);
    sheet->loaded = (sheet->texture.id != 0);
    if (!sheet->loaded)
        fprintf(stderr, "[sprite] could not load: %s\n", path);
}

static void DrawSprite(const SpriteSheet* sheet, unsigned frame, float x, float y)
{
    if (!sheet->loaded || frame >= sheet->count)
        return;

    const Sprite* s = &sheet->frames[frame];
    Rectangle src = { (float)s->srcX, (float)s->srcY, (float)s->srcW, (float)s->srcH };
    Rectangle dst = { x, y, (float)s->width, (float)s->height };
    DrawTexturePro(sheet->texture, src, dst, (Vector2) { 0.0f, 0.0f }, 0.0f, WHITE);
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

static void DrawCell(int x, int y, Color c)
{
    Rectangle rec = {
        .x = x * CELL_WIDTH,
        .y = HUD_HEIGHT + (y * CELL_HEIGHT),
        .width = CELL_WIDTH - 2, // Slight offset padding for cleaner appearance
        .height = CELL_HEIGHT - 2
    };
    DrawRectangleRec(rec, c);
}

static void DrawGRID(void)
{
    for (int i = 0; i <= COLS; i++) {
        DrawLine(i * CELL_WIDTH, HUD_HEIGHT, i * CELL_WIDTH, SCREEN_HEIGHT, COLOR_GRID);
    }
    for (int j = 0; j <= ROWS; j++) {
        DrawLine(0, HUD_HEIGHT + (j * CELL_HEIGHT), SCREEN_WIDTH, HUD_HEIGHT + (j * CELL_HEIGHT), COLOR_GRID);
    }
}

static void DrawHUD(const Game* game)
{
    DrawRectangle(0, 0, SCREEN_WIDTH, HUD_HEIGHT, BLACK);
    DrawLine(0, HUD_HEIGHT, SCREEN_WIDTH, HUD_HEIGHT, RAYWHITE);

    DrawText(TextFormat("STARS: %02d/%02d   SCORE: %04d", game->hits, game->total_bonus, game->score),
        20, 10, 20, RAYWHITE);

    if (game->state == STATE_PAUSE) {
        DrawText("PAUSED (Press 'P' to Resume)", SCREEN_WIDTH - 320, 10, 20, GOLD);
    } else if (game->state == STATE_WIN) {
        DrawText("CONGRATULATIONS! Press 'R' to Restart", SCREEN_WIDTH - 380, 10, 20, RED);
    } else if (game->state == STATE_GAME_OVER) {
        DrawText("GAME OVER! Press 'R' to Restart", SCREEN_WIDTH - 360, 10, 20, RED);
    } else {
        DrawText("WASD/Arrows: Move | R: Restart | P: Pause", SCREEN_WIDTH - 360, 12, 16, LIGHTGRAY);
    }
}

static bool IsInBounds(int x, int y)
{
    return x >= 0 && x < COLS && y >= 0 && y < ROWS;
}

static bool IsWalkable(const Game* game, int x, int y)
{
    return IsInBounds(x, y) && game->map[x][y] != CELL_OBSTACLE;
}

static bool IsOccupied(const Game* game, int x, int y)
{
    if (x == game->player.x && y == game->player.y)
        return true;
    if (x == game->enemy.x && y == game->enemy.y)
        return true;
    return game->map[x][y] != CELL_EMPTY;
}

static Point RandomEmptyCell(const Game* game)
{
    Point p;
    do {
        p.x = GetRandomValue(0, COLS - 1);
        p.y = GetRandomValue(0, ROWS - 1);
    } while (IsOccupied(game, p.x, p.y));
    return p;
}

// A* Pathfinding implementation targeting coordinates dynamically around solid blocks
static Direction Pathfinding(const Game* game, Point start, Point target)
{
    if (start.x == target.x && start.y == target.y)
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

        int dx[] = { 0, 0, -1, 1 };
        int dy[] = { -1, 1, 0, 0 };

        for (int i = 0; i < 4; i++) {
            int nx = currentX + dx[i];
            int ny = currentY + dy[i];

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

    // Fallback: Greedy straight-line movement if pathfinding is blocked
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
