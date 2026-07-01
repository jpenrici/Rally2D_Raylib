#include "raylib.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_TITLE "Raylib"
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define BORDER 25

#define TARGET_FPS 60

#define COLOR_BACKGROUND DARKGRAY
#define COLOR_SHAPE YELLOW
#define COLOR_TEXT GREEN

typedef struct {
    const char* label;
    void (*execute)(Vector2*);
} Behavior;

static bool safe_execute(const Behavior* b, Vector2* v);

// Functions of behaviors
static void f1(Vector2* v);
static void f2(Vector2* v);
static void f3(Vector2* v);
static void f4(Vector2* v);
static void f5(Vector2* v);
static void f6(Vector2* v);
static void f7(Vector2* v);

static Behavior behaviors[] = {
    { "horizontal to the right", f1 },
    { "diagonal downward      ", f2 },
    { "ping-pong              ", f3 },
    { "sine                   ", f4 },
    { "figure-eight shaped    ", f5 },
    { "random                 ", f6 },
    { "arrow (WASD)           ", f7 }
};

// Shape
static void DrawShape(const Vector2 v);

// Main
int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_TITLE);
    SetTargetFPS(TARGET_FPS);

    Vector2 v = { .x = SCREEN_WIDTH / 2, .y = SCREEN_HEIGHT / 2 };

    bool isRunning = true;

    const int SIZE = ((int)(sizeof(behaviors) / sizeof(behaviors[0])));

    bool clearScreen = true;

    int index = 0;

    while (!WindowShouldClose() && isRunning) {

        if (IsKeyPressed(KEY_Q))
            isRunning = false;

        if (IsKeyPressed(KEY_LEFT)) {
            index--;
            if (index < 0)
                index = SIZE - 1;
        }

        if (IsKeyPressed(KEY_RIGHT)) {
            index++;
            index %= SIZE;
        }

        if (IsKeyPressed(KEY_SPACE))
            clearScreen = !clearScreen;

        BeginDrawing();
        {
            if (clearScreen)
                ClearBackground(COLOR_BACKGROUND);

            safe_execute(&behaviors[index], &v);
            DrawShape(v);

            // UI Text
            DrawRectangle(0, 0, SCREEN_WIDTH, BORDER, COLOR_BACKGROUND);
            DrawText(behaviors[index].label, 10, 10, 20, COLOR_TEXT);
            DrawRectangle(0, SCREEN_HEIGHT - BORDER, SCREEN_WIDTH, BORDER, COLOR_BACKGROUND);

            char nav[64];
            snprintf(nav, sizeof(nav), "< %d / %u >", index + 1, SIZE);
            DrawText(nav, 10, 36, 16, GRAY);
            DrawText("[LEFT] previous  [RIGHT] next  [Q] exit", 10, SCREEN_HEIGHT - 24, 14, LIGHTGRAY);
        }
        EndDrawing();
    }

    CloseWindow();

    return 0;
}

static void DrawShape(Vector2 v)
{
    DrawCircleLinesV(v, 10, COLOR_SHAPE);
}

// Functions
static bool safe_execute(const Behavior* b, Vector2* v)
{
    if (!b || !b->execute)
        return false;

    b->execute(v);

    return true;
}

// f1: horizontal to the right
static void f1(Vector2* v)
{
    v->x += 4.0f;
    if (v->x > SCREEN_WIDTH)
        v->x = 0.0f;
}

// f2: diagonal downward
static void f2(Vector2* v)
{
    v->x += 3.0f;
    v->y += 2.0f;

    if (v->x > SCREEN_WIDTH)
        v->x = 0.0f;
    if (v->y > SCREEN_HEIGHT)
        v->y = 0.0f;
}

// f3: ping-pong (bounces off screen borders)
static void f3(Vector2* v)
{
    static Vector2 speed = { 4.0f, 4.0f };

    v->x += speed.x;
    v->y += speed.y;

    if (v->x >= SCREEN_WIDTH - BORDER || v->x <= BORDER)
        speed.x *= -1.0f;
    if (v->y >= SCREEN_HEIGHT - BORDER || v->y <= BORDER)
        speed.y *= -1.0f;
}

// f4: sine wave
static void f4(Vector2* v)
{
    v->x += 3.0f;
    if (v->x > SCREEN_WIDTH)
        v->x = 0.0f;

    // Smoothly calculate Y based on running time
    v->y = SCREEN_HEIGHT / 2 + sinf((float)GetTime() * 4.0f) * 100.0f;
}

// f5: figure-eight shaped (Lissajous curve / Lemniscate style)
static void f5(Vector2* v)
{
    float time = (float)GetTime() * 2.0f;
    // Standard infinity pattern parametric formula
    v->x = SCREEN_WIDTH / 2 + cosf(time) * 200.0f;
    v->y = SCREEN_HEIGHT / 2 + sinf(2.0f * time) * 100.0f / 2.0f;
}

// f6: random movement (drunken walk)
static void f6(Vector2* v)
{
    v->x += (float)GetRandomValue(-4, 4);
    v->y += (float)GetRandomValue(-4, 4);

    // Constrain to window bounds
    if (v->x < BORDER)
        v->x = BORDER;
    if (v->x > SCREEN_WIDTH - BORDER)
        v->x = SCREEN_WIDTH - BORDER;
    if (v->y < BORDER)
        v->y = BORDER;
    if (v->y > SCREEN_HEIGHT - BORDER)
        v->y = SCREEN_HEIGHT - BORDER;
}

// f7: WASD interactive movement
static void f7(Vector2* v)
{
    float speed = 5.0f;

    // Using IsKeyDown for fluid, continuous movement instead of tap-by-tap
    if (IsKeyDown(KEY_W) && v->y > BORDER) {
        v->y -= speed;
    }
    if (IsKeyDown(KEY_S) && v->y < SCREEN_HEIGHT - BORDER) {
        v->y += speed;
    }
    if (IsKeyDown(KEY_A) && v->x > BORDER) {
        v->x -= speed;
    }
    if (IsKeyDown(KEY_D) && v->x < SCREEN_WIDTH - BORDER) {
        v->x += speed;
    }
}
