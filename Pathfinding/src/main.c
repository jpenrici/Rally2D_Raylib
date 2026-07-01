#include "raylib.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// Obstacles
#define MIN_NUM 25
#define MAX_NUM 50

// Colors
#define COLOR_BACKGROUND DARKGRAY
#define COLOR_OBSTACLE RED
#define COLOR_PLAYER BLUE
#define COLOR_ENEMY GOLD
#define COLOR_GRID DARKGRAY

// Structures
typedef enum {
    PLAYER,
    ENEMY,
    OBSTACLE,
    UNDEFINED
} EntityKind;

typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    NONE
} Direction;

typedef struct {
    Vector2 position; // Grid coordinates (0 to COLS-1, 0 to ROWS-1)
    EntityKind kind;
    bool solid;
    int id;
} Entity;

typedef enum {
    STATE_RUNNING,
    STATE_PAUSE,
    STATE_GAME_OVER,
    STATE_QUIT
} GameState;

typedef struct {
    Entity player;
    Entity enemy;
    Entity* obstacles;
    int total_obstacles;
    GameState state;
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
static void GameQuit(Game* game);

static void DrawEntity(const Entity* e, Color c);
static void DrawGRID(void);
static void DrawHUD(const Game* game);

static bool CheckCollision(const Entity* e1, const Entity* e2);
static bool IsCellOccupiedByObstacle(const Game* game, int x, int y);
static Direction Pathfinding(const Game* game, const Entity* start_entity, const Entity* target_entity);

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

    GameQuit(&game);
    CloseWindow();

    return 0;
}

static void GameInit(Game* game)
{
    game->score = 0;
    game->frames_counter = 0;

    // Allocate and clear obstacle array safely
    if (game->obstacles != NULL) {
        free(game->obstacles);
        game->obstacles = NULL;
    }

    // Set Random Player Position (Safe within bounds: 0 to COLS-1)
    Entity* p = &game->player;
    p->kind = PLAYER;
    p->solid = true;
    p->position = (Vector2) { (float)GetRandomValue(0, COLS - 1), (float)GetRandomValue(0, ROWS - 1) };
    p->id = 1;

    // Set Random Enemy Position (Ensuring it does not spawn on top of the player)
    Entity* e = &game->enemy;
    e->kind = ENEMY;
    e->solid = true;
    e->id = 2;
    do {
        e->position = (Vector2) { (float)GetRandomValue(0, COLS - 1), (float)GetRandomValue(0, ROWS - 1) };
    } while (e->position.x == p->position.x && e->position.y == p->position.y);

    // Generate Random Obstacles
    game->total_obstacles = GetRandomValue(MIN_NUM, MAX_NUM);
    game->obstacles = malloc(sizeof(Entity) * game->total_obstacles);

    if (game->obstacles) {
        for (int i = 0; i < game->total_obstacles; ++i) {
            Entity o;
            o.kind = OBSTACLE;
            o.solid = true;
            o.id = i + 10;

            // Loop until a unique empty cell is found
            while (true) {
                o.position = (Vector2) { (float)GetRandomValue(0, COLS - 1), (float)GetRandomValue(0, ROWS - 1) };

                // Do not block Player or Enemy starting points
                if ((o.position.x == p->position.x && o.position.y == p->position.y) || (o.position.x == e->position.x && o.position.y == e->position.y)) {
                    continue;
                }

                // Check if this cell already has an obstacle
                bool overlap = false;
                for (int j = 0; j < i; ++j) {
                    if (game->obstacles[j].position.x == o.position.x && game->obstacles[j].position.y == o.position.y) {
                        overlap = true;
                        break;
                    }
                }
                if (!overlap)
                    break;
            }
            game->obstacles[i] = o;
        }
    }

    game->state = STATE_RUNNING;
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

    // Grid-based Player Movement Input (Includes collision check with solid obstacles)
    Vector2 next_pos = game->player.position;

    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP))
        next_pos.y--;
    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN))
        next_pos.y++;
    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT))
        next_pos.x--;
    if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT))
        next_pos.x++;

    // Map boundary constraint validation
    if (next_pos.x >= 0 && next_pos.x < COLS && next_pos.y >= 0 && next_pos.y < ROWS) {
        if (!IsCellOccupiedByObstacle(game, (int)next_pos.x, (int)next_pos.y)) {
            game->player.position = next_pos;
        }
    }
}

static void GameUpdate(Game* game)
{
    game->frames_counter++;

    // Throttle Enemy movement speed (e.g., Moves every 15 frames)
    if (game->frames_counter % 15 == 0) {
        Direction dir = Pathfinding(game, &game->enemy, &game->player);

        Vector2 next_enemy_pos = game->enemy.position;
        switch (dir) {
        case UP:
            next_enemy_pos.y--;
            break;
        case DOWN:
            next_enemy_pos.y++;
            break;
        case LEFT:
            next_enemy_pos.x--;
            break;
        case RIGHT:
            next_enemy_pos.x++;
            break;
        default:
            break; // No path found or reached target
        }

        // Prevent enemy from clipping into solid obstacles
        if (!IsCellOccupiedByObstacle(game, (int)next_enemy_pos.x, (int)next_enemy_pos.y)) {
            game->enemy.position = next_enemy_pos;
        }
    }
}

static void GamePlay(Game* game)
{
    // Collision condition with Enemy (Game Over)
    if (CheckCollision(&game->player, &game->enemy)) {
        game->state = STATE_GAME_OVER;
    }

    // Check collision with obstacles (Break them)
    for (int i = 0; i < game->total_obstacles; ++i) {
        if (game->obstacles[i].solid && CheckCollision(&game->player, &game->obstacles[i])) {
            game->obstacles[i].solid = false;
            game->score += 10; // Gain points for removing obstacles
        }
    }
}

static void GameRender(const Game* game)
{
    BeginDrawing();
    ClearBackground(COLOR_BACKGROUND);

    DrawGRID();

    // Render active obstacles
    for (int i = 0; i < game->total_obstacles; ++i) {
        if (game->obstacles[i].solid) {
            DrawEntity(&game->obstacles[i], COLOR_OBSTACLE);
        }
    }

    DrawEntity(&game->player, COLOR_PLAYER);
    DrawEntity(&game->enemy, COLOR_ENEMY);

    DrawHUD(game);

    EndDrawing();
}

static void GameQuit(Game* game)
{
    if (game->obstacles != NULL) {
        free(game->obstacles);
        game->obstacles = NULL;
    }
}

static void DrawEntity(const Entity* e, Color c)
{
    // Render entity within grid coordinates mapped to pixels
    Rectangle rec = {
        .x = e->position.x * CELL_WIDTH,
        .y = HUD_HEIGHT + (e->position.y * CELL_HEIGHT),
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
    // Background Top HUD Bar
    DrawRectangle(0, 0, SCREEN_WIDTH, HUD_HEIGHT, BLACK);
    DrawLine(0, HUD_HEIGHT, SCREEN_WIDTH, HUD_HEIGHT, RAYWHITE);

    // Score and Context status rendering
    DrawText(TextFormat("SCORE: %04d", game->score), 20, 10, 20, RAYWHITE);

    if (game->state == STATE_PAUSE) {
        DrawText("PAUSED (Press 'P' to Resume)", SCREEN_WIDTH / 2 - 140, 10, 20, GOLD);
    } else if (game->state == STATE_GAME_OVER) {
        DrawText("GAME OVER! Press 'R' to Restart", SCREEN_WIDTH / 2 - 150, 10, 20, RED);
    } else {
        DrawText("WASD/Arrows: Move | R: Restart | P: Pause", SCREEN_WIDTH - 420, 12, 16, LIGHTGRAY);
    }
}

static bool CheckCollision(const Entity* e1, const Entity* e2)
{
    return (e1->position.x == e2->position.x && e1->position.y == e2->position.y && e1->solid && e2->solid);
}

static bool IsCellOccupiedByObstacle(const Game* game, int x, int y)
{
    for (int i = 0; i < game->total_obstacles; ++i) {
        if (game->obstacles[i].solid && (int)game->obstacles[i].position.x == x && (int)game->obstacles[i].position.y == y) {
            return true;
        }
    }
    return false;
}

// A* Pathfinding implementation targeting coordinates dynamically around solid blocks
static Direction Pathfinding(const Game* game, const Entity* start_entity, const Entity* target_entity)
{
    int startX = (int)start_entity->position.x;
    int startY = (int)start_entity->position.y;
    int targetX = (int)target_entity->position.x;
    int targetY = (int)target_entity->position.y;

    if (startX == targetX && startY == targetY)
        return NONE;

    // Allocate 2D map graph grid nodes
    Node grid[COLS][ROWS];
    for (int x = 0; x < COLS; x++) {
        for (int y = 0; y < ROWS; y++) {
            grid[x][y].x = x;
            grid[x][y].y = y;
            grid[x][y].walkable = !IsCellOccupiedByObstacle(game, x, y);
            grid[x][y].g_cost = 1000000; // Simulating Infinity
            grid[x][y].h_cost = abs(x - targetX) + abs(y - targetY); // Manhattan distance heuristic
            grid[x][y].f_cost = 1000000;
            grid[x][y].parent = NULL;
        }
    }

    // Explicitly open lists tracking structures
    bool open_set[COLS][ROWS] = { false };
    bool closed_set[COLS][ROWS] = { false };

    // Setup initial node
    grid[startX][startY].g_cost = 0;
    grid[startX][startY].f_cost = grid[startX][startY].h_cost;
    open_set[startX][startY] = true;

    bool path_found = false;

    while (true) {
        // Find node with lowest f_cost in open set
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

        // Check if destination reached
        if (currentX == targetX && currentY == targetY) {
            path_found = true;
            break;
        }

        open_set[currentX][currentY] = false;
        closed_set[currentX][currentY] = true;

        // Process 4 connected neighbors (Up, Down, Left, Right)
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

    // Trace path back to find the first step
    if (path_found) {
        Node* curr = &grid[targetX][targetY];
        while (curr->parent != NULL && (curr->parent->x != startX || curr->parent->y != startY)) {
            curr = curr->parent;
        }

        if (curr->x < startX)
            return LEFT;
        if (curr->x > startX)
            return RIGHT;
        if (curr->y < startY)
            return UP;
        if (curr->y > startY)
            return DOWN;
    }

    // Fallback: Greedy straight-line movement if pathfinding is blocked
    if (startX < targetX && !IsCellOccupiedByObstacle(game, startX + 1, startY))
        return RIGHT;
    if (startX > targetX && !IsCellOccupiedByObstacle(game, startX - 1, startY))
        return LEFT;
    if (startY < targetY && !IsCellOccupiedByObstacle(game, startX, startY + 1))
        return DOWN;
    if (startY > targetY && !IsCellOccupiedByObstacle(game, startX, startY - 1))
        return UP;

    return NONE;
}
