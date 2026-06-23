#pragma once

#include "raylib.h"

#include <stdbool.h>

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
