#include "raylib.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_TITLE "Graphics"
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define TIMER_FPS 60

#define ORIGIN_X (SCREEN_WIDTH / 2)
#define ORIGIN_Y (SCREEN_HEIGHT / 2)
#define SCALE 60.0f

static inline float lx(float x) { return ORIGIN_X + x * SCALE; }
static inline float ly(float y) { return ORIGIN_Y - y * SCALE; }

typedef enum {
    STATE_RUNNING,
    STATE_QUIT
} State;

typedef struct {
    State state;
    int current; // current graph index
    int total; // total number of graphs
} Window;

static void HandleInput(Window* win);
static void DrawAxes(void);

typedef struct {
    const char* label;
    void (*execute)(void);
} Draw;

static bool safe_execute(const Draw* d);

// Samples
static void f1(void);
static void f2(void);
static void f3(void);

static Draw graphs[] = {
    { "f(x) = sin(x)", f1 },
    { "f(x) = cos(x)", f2 },
    { "Ellipse: x=3cos(t), y=2sin(t)", f3 },
};

#define GRAPH_COUNT ((int)(sizeof(graphs) / sizeof(graphs[0])))

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_TITLE);
    SetTargetFPS(TIMER_FPS);

    Window win = (Window) {
        .state = STATE_RUNNING,
        .current = 0,
        .total = GRAPH_COUNT
    };

    while (!WindowShouldClose() && win.state != STATE_QUIT) {
        HandleInput(&win);

        BeginDrawing();
        {
            ClearBackground(WHITE);
            DrawAxes();
            safe_execute(&graphs[win.current]);
            DrawText(graphs[win.current].label, 10, 10, 20, DARKBLUE);

            char nav[64];
            snprintf(nav, sizeof(nav), "< %d / %d >", win.current + 1, win.total);
            DrawText(nav, 10, 36, 16, GRAY);
            DrawText("[LEFT] previous  [RIGHT] next  [Q] exit", 10, SCREEN_HEIGHT - 24, 14, LIGHTGRAY);
        }
        EndDrawing();
    }

    CloseWindow();

    return 0;
}

static void HandleInput(Window* win)
{
    if (IsKeyPressed(KEY_Q))
        win->state = STATE_QUIT;

    if (IsKeyPressed(KEY_RIGHT))
        win->current = (win->current + 1) % win->total;

    if (IsKeyPressed(KEY_LEFT))
        win->current = (win->current - 1 + win->total) % win->total;
}

static void DrawAxes(void)
{
    DrawLine(0, ORIGIN_Y, SCREEN_WIDTH, ORIGIN_Y, LIGHTGRAY);
    DrawLine(ORIGIN_X, 0, ORIGIN_X, SCREEN_HEIGHT, LIGHTGRAY);

    for (int u = -(ORIGIN_X / (int)SCALE) - 1; u <= (ORIGIN_X / (int)SCALE) + 1; u++) {
        if (u == 0)
            continue;
        int px = (int)lx((float)u);
        DrawLine(px, ORIGIN_Y - 4, px, ORIGIN_Y + 4, GRAY);

        char buf[8];
        snprintf(buf, sizeof(buf), "%d", u);
        DrawText(buf, px - 5, ORIGIN_Y + 7, 12, GRAY);
    }

    for (int u = -(ORIGIN_Y / (int)SCALE) - 1; u <= (ORIGIN_Y / (int)SCALE) + 1; u++) {
        if (u == 0)
            continue;
        int py = (int)ly((float)u);
        DrawLine(ORIGIN_X - 4, py, ORIGIN_X + 4, py, GRAY);

        char buf[8];
        snprintf(buf, sizeof(buf), "%d", u);
        DrawText(buf, ORIGIN_X + 7, py - 6, 12, GRAY);
    }

    DrawText("0", ORIGIN_X + 5, ORIGIN_Y + 5, 12, GRAY);
}

// Functions
static bool safe_execute(const Draw* d)
{
    if (!d || !d->execute)
        return false;

    d->execute();

    return true;
}

// f1 — sino: y = sin(x)
static void f1(void)
{
    int steps = SCREEN_WIDTH * 2;
    float x_min = -(float)ORIGIN_X / SCALE;
    float x_max = (float)(SCREEN_WIDTH - ORIGIN_X) / SCALE;
    float dx = (x_max - x_min) / (float)steps;

    for (int i = 0; i < steps - 1; i++) {
        float x0 = x_min + i * dx;
        float x1 = x_min + (i + 1) * dx;
        float y0 = sinf(x0);
        float y1 = sinf(x1);

        DrawLine((int)lx(x0), (int)ly(y0), (int)lx(x1), (int)ly(y1), RED);
    }
}

// f2 — cosino: y = cos(x)
static void f2(void)
{
    int steps = SCREEN_WIDTH * 2;
    float x_min = -(float)ORIGIN_X / SCALE;
    float x_max = (float)(SCREEN_WIDTH - ORIGIN_X) / SCALE;
    float dx = (x_max - x_min) / (float)steps;

    for (int i = 0; i < steps - 1; i++) {
        float x0 = x_min + i * dx;
        float x1 = x_min + (i + 1) * dx;
        float y0 = cosf(x0);
        float y1 = cosf(x1);

        DrawLine((int)lx(x0), (int)ly(y0), (int)lx(x1), (int)ly(y1), BLUE);
    }
}

// f3 — parametric ellipse: x(t)=3cos(t), y(t)=2sin(t)
static void f3(void)
{
    int steps = 720;

    for (int i = 0; i < steps; i++) {
        float t0 = (float)i / (float)steps * 2.0f * PI;
        float t1 = (float)(i + 1) / (float)steps * 2.0f * PI;

        float x0 = 3.0f * cosf(t0), y0 = 2.0f * sinf(t0);
        float x1 = 3.0f * cosf(t1), y1 = 2.0f * sinf(t1);

        DrawLine((int)lx(x0), (int)ly(y0), (int)lx(x1), (int)ly(y1), DARKGREEN);
    }
}
