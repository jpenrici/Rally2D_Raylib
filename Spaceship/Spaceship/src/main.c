/*
 * A simple spaceship game where the player must hit meteors with
 * a bullet while avoiding collisions. The HUD will display a timer,
 * points earned from destroying meteors, hits, shots fired,
 * the number of collisions, and energy levels.
 *
 * Rendering will take place in layers, following this sequence:
 * 1) background,
 * 2) meteors ("extras"),
 * 3) meteors (targets),
 * 4) player and crosshair,
 * 5) and HUD.
 */

#include "raylib.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

// Window
#define SCREEN_TITLE "Spaceship"
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

// FPS
#define TIMER_FPS 60 // target frames per second

// Background
#define BG_PATH "resources/images/background.png"
#define BG_WIDTH SCREEN_WIDTH
#define BG_HEIGHT SCREEN_HEIGHT

// Player - Spaceship
#define PLAYER_PATH "resources/images/spaceship.png"
#define PLAYER_WIDTH 128
#define PLAYER_HEIGHT 256
#define PLAYER_ROWS 1
#define PLAYER_COLS 6
#define PLAYER_MOVE_HORIZONTAL 4
#define PLAYER_MOVE_VERTICAL 0

#define PLAYER_ENERGY_MIN 0
#define PLAYER_ENERGY_MAX 10

// Bullet - Spaceship
#define BULLET_PATH "resources/images/bullet.png"
#define BULLET_WIDTH 128
#define BULLET_HEIGHT 256
#define BULLET_ROWS 1
#define BULLET_COLS 2
#define BULLET_MOVE_HORIZONTAL 4
#define BULLET_MOVE_VERTICAL 0

// Obstacle - Meteor
#define OBSTACLE_PATH "resources/images/meteor.png"
#define OBSTACLE_WIDTH 192
#define OBSTACLE_HEIGHT 192
#define OBSTACLE_ROWS 1
#define OBSTACLE_COLS 6
#define OBSTACLE_MOVE_HORIZONTAL 4
#define OBSTACLE_MOVE_VERTICAL 0

// Game Over
#define GAMEOVER_PATH "resources/images/gameOver.png"

// Fallback
#define COLOR_PLAYER_FALLBACK (Color) { 220, 50, 50, 255 } // red
#define COLOR_BULLET_FALLBACK (Coloe) { 0, 200, 80, 255 } // green
#define COLOR_OBSTACLE_FALLBACK (Color) { 76, 63, 47, 255 } // dark Brown

// Audio
#define MUSIC_PATH "resources/audio/music.mp3"
#define ALERT_PATH "resources/audio/alert.mp3"
#define EXPLOSION_PATH "resources/audio/explosion.mp3"
#define SHOT_PATH "resources/audio/shot.mp3"

// Fonts
#define FONT1 "resources/fonts/NotoSans-Black.ttf"

// ---------------------------------------------------------------------------
// Types
// ---------------------------------------------------------------------------

typedef struct {
    unsigned srcX; // source X offset inside the texture (pixels)
    unsigned srcY; // source Y offset inside the texture (pixels)
    unsigned srcW; // source rectangle width
    unsigned srcH; // source rectangle height
    unsigned width; // image width
    unsigned height; // image height
} Sprite;

typedef struct {
    Texture2D texture; // image with the Sprites
    Sprite* frames; // array of [rows * cols] Sprite values
    unsigned count; // total number of frames (rows * cols)
    bool loaded; // if false, use fallback shape
} SpriteSheet;

typedef struct {
    Texture2D texture;
    float scrollOffset; // vertical pixel offset, wraps at heigth
    bool loaded; // if false, draw a solid color instead
} ScrollBackground;

typedef struct {
    SpriteSheet sheet;
    Vector2 pos;
    float angle;
    unsigned energy;
    unsigned frame;
} Player;

typedef struct {
    SpriteSheet sheet;
    Vector2 pos;
    float angle;
    float scale;
    unsigned frame;
    unsigned animTimer;
} Bullet;

typedef struct {
    SpriteSheet sheet;
    Vector2 pos;
    float angle;
    unsigned frame;
} Obstacle;

typedef struct {
    int timer;
    int score;
    int hits;
    int shots;
    int collisions;
    unsigned energy;
} HUD;

typedef struct {
    Texture2D texture;
    bool imageLoaded;
} Image;

typedef enum {
    STATE_PLAYING,
    STATE_PAUSE,
    STATE_WIN,
    STATE_GAMEOVER,
    STATE_QUIT
} GameState;

typedef struct {
    Player spaceship;
    Obstacle* meteors; // array
    ScrollBackground background;
    Hud hud;
    Image gameOver;
    GameState state;
} Game;

// ---------------------------------------------------------------------------
// Prototypes
// ---------------------------------------------------------------------------

static void GameInit(Game* game);
static void GameHandleInput(Game* game);
static void GameUpdate(Game* game);
static void GamePlay(Game* game);
static void GameRender(const Game* game);
static void GameCheckCollisions(Game* game);
static void GameQuit(Game* game);

void PrepareSheet(SpriteSheet* sheet, const char* path, unsigned rows, unsigned cols, unsigned frameW, unsigned frameH);
void DrawSprite(const SpriteSheet* sheet, unsigned frame, float x, float y);
void DrawSpriteFallback(const SpriteSheet* sheet, unsigned frame, float x, float y, Color color);
void AnimateSprite(unsigned* frame, int* timer, unsigned firstFrame, unsigned lastFrame, int ticksPerFrame);
void ResizeSprite(SpriteSheet* sheet, unsigned newW, unsigned newH);
void UnloadSheet(SpriteSheet* sheet);

void LoadAllAssets(Game* game, int screenW, int screenH);
void UnloadAllAssets(Game* game);

void InitScroll(ScrollBackground* bg);
void UpdateScroll(ScrollBackground* bg, float speed);

static void RenderFrame(const Game* game, int screeWidth, int screenHeight);
static void DrawBackground(Game* game);
static void DrawPlayer(Game* game);
static void DrawPlayerTarget(Game* game);
static void DrawPlayerHud(Game* game);
static void DrawBullet(Game* game);
static void DrawObstacle(Game* game);
static void DrawHud(Game* game);
static void DrawGameOver(const Game* game, int screeWidth, int screenHeight);

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------

int main(void)
{
    // Window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_TITLE);
    SetTargetFPS(TIMER_FPS);

    // Initialize
    Game game = { 0 };
    GameInit(&game, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Game Loop
    while (!WindowShouldClose() && game.state != STATE_QUIT) {
        // Keyboard and Mouse Events
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

// ---------------------------------------------------------------------------
// Game logic
// ---------------------------------------------------------------------------

static void GameInit(Game* game)
{
    // Player
    Player* spaceship = &game->spaceship;

    // Obstacle

    // Bullet

    // GameOver

    // Fonts
    // Hud

    // Audio

    game->state = STATE_PLAYING;
}

static void GameHandleInput(Game* game)
{
    // Pause / resume
    if (IsKeyPressed(KEY_P)) {
        if (game->state == STATE_PLAYING) {
            // TO DO
            game->state = STATE_PAUSE;
        } else if (game->state == STATE_PAUSE) {
            // TO DO
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

    // Game controls
    Player* spaceship = &game->spaceship;

    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
        // TO DO
    }

    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
        // TO DO
    }

    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
        // TO DO
    }

    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
        // TO DO
    }

    if (game->state == STATE_PLAYING && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        // TO DO
        // SHOT
    }
}

static void GameUpdate(Game* game)
{
    // Gameplay - Rules
    // HUD
}

static void GameRender(const Game* game)
{
    // Playing area background

    // Objects
    // Drawing set of obstacles

    // Draw bullet if fired.

    // Draw Player and trigger

    // DrawHud(game);

    if (game->state == STATE_PAUSE)
        DrawOverlay("PAUSED  [P to resume]", (Color) { 0, 0, 0, 160 }, RAYWHITE);
}

static void GamePlay(Game* game)
{
    // Move Player (x,y) and angle (rotation)
    // Check trigger
    // Move main obstacles and extras.
    // Check collisions
}

static void GameQuit(Game* game)
{
    // Free and quit...
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

void PrepareSheet(SpriteSheet* sheet, const char* path, unsigned rows, unsigned cols,
    unsigned frameW, unsigned frameH)
{
    unsigned count = rows * cols;

    sheet->frames = (Sprite*)malloc(sizeof(Sprite) * (size_t)count);
    sheet->count = count;

    if (sheet->frames == NULL) {
        fprintf(stderr, "[sprite] malloc failed fo %s\n", path);
        sheet->loaded = false;
        return;
    }

    unsigned idx = 0;
    for (unsigned i = 0; i < rows; ++i) {
        for (unsigned j = 0; j < cols; ++j) {
            Sprite* s = &sheet->frames[idx++];
            s->srcX = j * frameW;
            s->srcY = i * frameH;
            s->srcW = frameW;
            s->srcH = frameH;
            s->width = frameW;
            s->height = frameH;
        }
    }

    sheet->texture = LoadTexture(path);
    sheet->loaded = (sheet->texture.id != 0);

    if (!sheet->loaded) {
        fprintf(stderr, "[sprite] could not load texture: %s\n", path);
    }
}

void DrawSprite(const SpriteSheet* sheet, unsigned frame, float x, float y)
{
    if (frame >= sheet->count)
        return;

    if (!sheet->loaded)
        return;

    const Stripe* s = &sheet->frames[frame];

    Rectangle src = { (int)s->srcX, (int)s->srcY, (float)s->srcW, (float)s->srcH };
    Rectangle dst = { x, y, (float)s->width, (float)s->height };

    Vector2 origin = { 0.0f, 0.0f };

    DrawTexturePro(sheet->texture, src, dst, origin, 0.0f, WHITE);
}

void DrawSpriteFallback(const SpriteSheet* sheet, unsigned frame, float x, float y, Color color)
{
    if (frame >= sheet->count)
        return;

    const Sprite* s = &sheet->frames[frame];

    DrawRectangle((int)x, (int)y, s->width, s->height, color);
    DrawRectangleLines((int)x, (int)y, s->width, s->height, BLACK);
}

void AnimateSprite(unsigned* frame, int* timer, unsigned firstFrame, unsigned lastFrame, int ticksPerFrame)
{
    (*timer)++;

    if (*timer > ticksPerFrame) {
        *timer = 0;
        (*frame)++;

        if (*frame > lastFrame) {
            *frame = firstFrame;
        }
    }
}

void ResizeSprite(SpriteSheet* sheet, unsigned newW, unsigned newH)
{
    for (unsigned i = 0; i < sheet->count; i++) {
        if (newW > 0)
            sheet->frames[i].width = newW;
        if (newH > 0)
            sheet->frames[i].height = newH;
    }
}

void UnloadSheet(SpriteSheet* sheet)
{
    free(sheet->frames);
    sheet->frames = NULL;
    sheet->count = 0;

    if (sheet->loaded) {
        UnloadTexture(sheet->texture);
        sheet->loaded = false;
    }
}

void LoadAllAssets(Game* game, int screenW, int screenH)
{
    // Player
    PrepareSheet(&game->spaceship.sheet, PLAYER_PATH, PLAYER_ROWS, PLAYER_COLS, PLAYER_WIDTH, PLAYER_HEIGHT);

    // Obstacle - Meteor
    // prepare a meteor
}

void UnloadAllAssets(Game* game)
{
    // Destroy all objects.
}

void InitScroll(ScrollBackground* bg);
void UpdateScroll(ScrollBackground* bg, float speed);

static void RenderFrame(const Game* game, int screeWidth, int screenHeight);
static void DrawBackground(Game* game);
static void DrawPlayer(Game* game);
static void DrawPlayerTarget(Game* game);
static void DrawPlayerHud(Game* game);
static void DrawBullet(Game* game);
static void DrawObstacle(Game* game);
static void DrawHud(Game* game);

static void DrawGameOver(const Game* game, int screeWidth, int screenHeight)
{
    if (game->gameOver.imageLoaded) {
        float x = (float)(screenW - gameOver.texture.width) * 0.5f;
        float y = (float)(screenH - gameOver.texture.height) * 0.5f;
        DrawTexture(game->gameOver.texture, (int)x, (int)y, WHITE);
    } else {
        int pw = 360, ph = 120;
        int px = (screenW - pw) / 2;
        int py = (screenH - ph) / 2;

        DrawRectangle(px, py, pw, ph, BLACK);
        DrawRectangleLines(px, py, pw, ph, RED);

        const char* msg = "GAME OVER";
        const char* hint = "Press SPACE to restart";
        int fontSize = 48;
        int hintSize = 20;

        DrawText(msg,
            px + (pw - MeasureText(msg, fontSize)) / 2,
            py + 18,
            fontSize, RED);
        DrawText(hint,
            px + (pw - MeasureText(hint, hintSize)) / 2,
            py + ph - hintSize - 14,
            hintSize, LIGHTGRAY);
    }
}
