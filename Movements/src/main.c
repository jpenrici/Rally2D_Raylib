#include "raylib.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_TITLE "Raylib"
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define BORDER 36

#define TARGET_FPS_DEFAULT 60
#define TARGET_FPS_MIN 1
#define TARGET_FPS_MAX 240
#define TARGET_FPS_STEP 5

#define COLOR_BACKGROUND DARKGRAY
#define COLOR_BORDER WHITE
#define COLOR_SHAPE YELLOW
#define COLOR_TEXT GREEN

typedef struct {
    const char* label;
    void (*execute)(Vector2*); // function pointer
} Behavior;

static bool safe_execute(const Behavior* b, Vector2* v);

// Functions of behaviors
#define BEHAVIOR_LIST                \
    X(f1, "horizontal to the right") \
    X(f2, "diagonal downward")       \
    X(f3, "ping-pong")               \
    X(f4, "sine                   ") \
    X(f5, "figure-eight shaped    ") \
    X(f6, "random                 ") \
    X(f7, "spiral                 ") \
    X(f8, "orbit                  ") \
    X(f9, "pendulum               ") \
    X(f10, "gravity bounce        ") \
    X(f11, "spring (follows mouse)") \
    X(fn, "arrow (WASD)           ")

#define X(fn, label) static void fn(Vector2* v);
BEHAVIOR_LIST
#undef X

#define X(fn, label) { label, fn },
static Behavior behaviors[] = { BEHAVIOR_LIST };
#undef X

// Shape
static void DrawShape(const Vector2 v);

// Main
int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_TITLE);

    int targetFPS = TARGET_FPS_DEFAULT;
    SetTargetFPS(targetFPS);

    int index = 0;
    const int SIZE = ((int)(sizeof(behaviors) / sizeof(behaviors[0])));

    bool clearScreen = true;

    Vector2 vec = { .x = SCREEN_WIDTH / 2, .y = SCREEN_HEIGHT / 2 };

    RenderTexture2D canvas = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
    ClearBackground(BLANK);

    while (!WindowShouldClose()) {

        if (IsKeyPressed(KEY_Q))
            break;

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

        // FPS control: UP/DOWN adjust the target FPS, R resets it to default.
        // This lets the user slow down or speed up the simulation to better
        // observe each movement behavior (e.g. lower FPS to study a single
        // step of the pendulum or gravity bounce).
        if (IsKeyPressed(KEY_UP)) {
            targetFPS += TARGET_FPS_STEP;
            if (targetFPS > TARGET_FPS_MAX)
                targetFPS = TARGET_FPS_MAX;
            SetTargetFPS(targetFPS);
        }

        if (IsKeyPressed(KEY_DOWN)) {
            targetFPS -= TARGET_FPS_STEP;
            if (targetFPS < TARGET_FPS_MIN)
                targetFPS = TARGET_FPS_MIN;
            SetTargetFPS(targetFPS);
        }

        if (IsKeyPressed(KEY_R)) {
            targetFPS = TARGET_FPS_DEFAULT;
            SetTargetFPS(targetFPS);
        }

        // Prepare texture for rendering
        BeginTextureMode(canvas);
        {
            if (clearScreen)
                ClearBackground(COLOR_BACKGROUND);
            safe_execute(&behaviors[index], &vec);
            DrawShape(vec);
        }
        EndTextureMode();

        BeginDrawing();
        {
            // BG
            ClearBackground(COLOR_BACKGROUND);

            // Texture
            DrawTextureRec(canvas.texture, (Rectangle) { 0, 0, SCREEN_WIDTH, -SCREEN_HEIGHT }, (Vector2) { 0, 0 }, WHITE);

            // Border
            DrawRectangleLines(BORDER, BORDER, SCREEN_WIDTH - 2 * BORDER, SCREEN_HEIGHT - 2 * BORDER, COLOR_BORDER);
            DrawRectangle(0, 0, SCREEN_WIDTH, BORDER, COLOR_BACKGROUND);
            DrawRectangle(0, 0, BORDER, SCREEN_HEIGHT, COLOR_BACKGROUND);
            DrawRectangle(SCREEN_WIDTH - BORDER, 0, BORDER, SCREEN_HEIGHT, COLOR_BACKGROUND);
            DrawRectangle(0, SCREEN_HEIGHT - BORDER, SCREEN_WIDTH, BORDER, COLOR_BACKGROUND);

            // Text
            DrawText(behaviors[index].label, 10, 2, 18, COLOR_TEXT);

            char nav[64];
            snprintf(nav, sizeof(nav), "< %d / %u >", index + 1, SIZE);

            DrawText(nav, 10, 22, 12, GRAY);
            DrawText("[LEFT] previous  [RIGHT] next  [UP/DOWN] FPS  [R] reset FPS  [Q] exit", 10, SCREEN_HEIGHT - 20, 14, LIGHTGRAY);

            char fpsInfo[64];
            snprintf(fpsInfo, sizeof(fpsInfo), "Target: %d  Actual: %d  Frame: %.2f ms",
                targetFPS, GetFPS(), GetFrameTime() * 1000.0f);
            DrawText(fpsInfo, SCREEN_WIDTH - 280, SCREEN_HEIGHT - 20, 14, LIGHTGRAY);
        }
        EndDrawing();
    }

    CloseWindow();

    return 0;
}

static void DrawShape(Vector2 v)
{
    float radius = 10.0f;
    DrawCircleLinesV(v, radius, COLOR_SHAPE);
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
    if (v->x > SCREEN_WIDTH - BORDER)
        v->x = (float)BORDER;
}

// f2: diagonal downward
static void f2(Vector2* v)
{
    v->x += 3.0f;
    v->y += 2.0f;

    if (v->x > SCREEN_WIDTH - BORDER)
        v->x = (float)BORDER;
    if (v->y > SCREEN_HEIGHT - BORDER)
        v->y = (float)BORDER;
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
    if (v->x > SCREEN_WIDTH - BORDER)
        v->x = (float)BORDER;

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
        v->x = (float)BORDER;
    if (v->x > SCREEN_WIDTH - BORDER)
        v->x = (float)(SCREEN_WIDTH - BORDER);
    if (v->y < BORDER)
        v->y = (float)BORDER;
    if (v->y > SCREEN_HEIGHT - BORDER)
        v->y = (float)(SCREEN_HEIGHT - BORDER);
}

// f7: spiral (grows outward from the center, then resets)
static void f7(Vector2* v)
{
    static float angle = 0.0f;
    static float radius = 0.0f;

    angle += 0.05f;
    radius += 0.3f;

    v->x = SCREEN_WIDTH / 2 + cosf(angle) * radius;
    v->y = SCREEN_HEIGHT / 2 + sinf(angle) * radius;

    // Reset once the spiral reaches the usable area, so it loops forever
    float maxRadius = (SCREEN_HEIGHT / 2) - BORDER;
    if (radius > maxRadius) {
        radius = 0.0f;
    }
}

// f8: orbit (steady circular motion around the center)
static void f8(Vector2* v)
{
    static float angle = 0.0f;
    const float radius = 150.0f;

    angle += 0.04f;

    v->x = SCREEN_WIDTH / 2 + cosf(angle) * radius;
    v->y = SCREEN_HEIGHT / 2 + sinf(angle) * radius;
}

// f9: pendulum (damped harmonic oscillator swinging from a fixed pivot)
static void f9(Vector2* v)
{
    static float angle = 1.2f; // initial angle from vertical, in radians
    static float angularVel = 0.0f;

    const float gravity = 0.006f;
    const float length = 180.0f;
    const float damping = 0.999f; // small energy loss per frame
    const Vector2 pivot = { SCREEN_WIDTH / 2.0f, BORDER + 40.0f };

    float angularAccel = -gravity / length * sinf(angle);
    angularVel = (angularVel + angularAccel) * damping;
    angle += angularVel;

    v->x = pivot.x + sinf(angle) * length;
    v->y = pivot.y + cosf(angle) * length;
}

// f10: gravity bounce (projectile falling under gravity, bouncing off
// the floor and side walls, losing a bit of energy on each bounce)
static void f10(Vector2* v)
{
    static Vector2 velocity = { 3.5f, 0.0f };

    const float gravity = 0.25f;
    const float bounceDamping = 0.85f;

    velocity.y += gravity;
    v->x += velocity.x;
    v->y += velocity.y;

    if (v->y >= SCREEN_HEIGHT - BORDER) {
        v->y = SCREEN_HEIGHT - BORDER;
        velocity.y *= -bounceDamping;
    }

    if (v->x <= BORDER || v->x >= SCREEN_WIDTH - BORDER) {
        velocity.x *= -1.0f;
        v->x = (v->x <= BORDER) ? (float)BORDER : (float)(SCREEN_WIDTH - BORDER);
    }
}

// f11: spring (elastically follows the mouse cursor, like a mass on a spring)
static void f11(Vector2* v)
{
    static Vector2 velocity = { 0.0f, 0.0f };

    const float stiffness = 0.02f;
    const float damping = 0.9f;

    Vector2 target = GetMousePosition();

    // Clamp target inside the border so the shape never leaves the frame
    if (target.x < BORDER)
        target.x = (float)BORDER;
    if (target.x > SCREEN_WIDTH - BORDER)
        target.x = (float)(SCREEN_WIDTH - BORDER);
    if (target.y < BORDER)
        target.y = (float)BORDER;
    if (target.y > SCREEN_HEIGHT - BORDER)
        target.y = (float)(SCREEN_HEIGHT - BORDER);

    Vector2 delta = { target.x - v->x, target.y - v->y };
    velocity.x = (velocity.x + delta.x * stiffness) * damping;
    velocity.y = (velocity.y + delta.y * stiffness) * damping;

    v->x += velocity.x;
    v->y += velocity.y;
}

// fn - last function: WASD interactive movement
static void fn(Vector2* v)
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
