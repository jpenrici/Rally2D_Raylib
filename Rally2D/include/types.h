#pragma once

#include "raylib.h"

#include <stdbool.h>

// Window
#define SCREEN_TITLE "Rally 2D"
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define BORDER_LEFT 100
#define BORDER_RIGHT 550

// FPS
#define TARGET_FPS 60

// Background
#define BG_PATH "assets/background.png"
#define BG_WIDTH 800
#define BG_HEIGHT 600

// Player - Car
#define PLAYER_PATH "assets/player-sprite.png"
#define PLAYER_WIDTH 128
#define PLAYER_HEIGHT 256
#define PLAYER_ROWS 1
#define PLAYER_COLS 6
#define PLAYER_MOVE_HORIZONTAL 4
#define PLAYER_MOVE_VERTICAL 0

#define VEL_MAX 60.0f
#define VEL_MIN 0.0f
#define BRAKING_SPEED 5.0f

// Obstacle
#define OBSTACLE_PATH "assets/obstacle-sprite.png"
#define OBSTACLE_WIDTH 192
#define OBSTACLE_HEIGHT 192
#define OBSTACLE_ROWS 1
#define OBSTACLE_COLS 6

// Speedometer
#define SPEEDOMETER_PATH "assets/speedometer-sprite.png"
#define SPEEDOMETER_WIDTH 128
#define SPEEDOMETER_HEIGHT 128
#define SPEEDOMETER_ROWS 7
#define SPEEDOMETER_COLS 10
#define SPEEDOMETER_MIN 0
#define SPEEDOMETER_MAX 60

// Ometer
#define ODOMETER_PATH "assets/odometer-sprite.png"
#define ODOMETER_WIDTH 34
#define ODOMETER_HEIGHT 20
#define ODOMETER_ROWS 50
#define ODOMETER_COLS 10
#define ODOMETER_MIN 0
#define ODOMETER_MAX 500

// Energy
#define ENERGY_PATH "assets/energy-sprite.png"
#define ENERGY_WIDTH 48
#define ENERGY_HEIGHT 128
#define ENERGY_ROWS 1
#define ENERGY_COLS 11
#define ENERGY_MIN 0
#define ENERGY_MAX 10

// Arrow
#define ARROW_PATH "assets/arrow-sprite.png"
#define ARROW_WIDTH 64
#define ARROW_HEIGHT 64
#define ARROW_ROWS 1
#define ARROW_COLS 5

// Game Over
#define GAMEOVER_PATH "assets/gameOver.png"

// Fallback
#define COLOR_CAR_FALLBACK (Color) { 220, 50, 50, 255 } // red
#define COLOR_OBSTACLE_FALLBACK (Color) { 76, 63, 47, 255 } // Dark Brown
#define COLOR_ENERGY_FALLBACK (Color) { 0, 200, 80, 255 } // green
#define COLOR_SPEED_FALLBACK (Color) { 30, 144, 255, 255 } // blue
#define COLOR_ODOM_FALLBACK (Color) { 255, 165, 0, 255 } // orange
#define COLOR_ARROW_FALLBACK (Color) { 255, 255, 255, 255 } // white

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

typedef struct {
    SpriteSheet sheet;
    Vector2 pos; // top-left position on screen (x, y)
    float velocity; // current speed  — range [0, 60]
    float step; // horizontal move per key press

    int energy; // remaining lives / energy — starts at 10
    int distance; // obstacles dodged / passed (odometer value)
    int frame; // current animation frame index
    int animTimer; // counts frames before advancing the animation
} Car;

typedef struct {
    SpriteSheet sheet;
    Vector2 pos;
    int frame;
} Obstacle;

typedef struct {
    Texture2D texture;
    float scrollOffset; // vertical pixel offset, wraps at heigth
    bool loaded; // if false, draw a solid color instead
} ScrollBackground;

typedef struct {
    SpriteSheet energy;
    SpriteSheet speedometer;
    SpriteSheet odometer;
    SpriteSheet arrow;
    int arrowFrame;
} HUD;

typedef enum {
    STATE_PLAYING,
    STATE_GAMEOVER,
    STATE_QUIT
} GameState;

typedef struct {
    Car car;
    Obstacle obstacle;
    ScrollBackground background;
    HUD hud;
    Texture2D gameOverTexture;
    bool gameOverLoaded;
    GameState state;
} Game;
