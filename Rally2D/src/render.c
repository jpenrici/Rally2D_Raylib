#include "raylib.h"

#include "background.h"
#include "render.h"
#include "sprite.h"

#define COL_CAR_FALLBACK (Color) { 220, 50, 50, 255 } /* red        */
#define COL_OBSTACLE_FALLBACK (Color) { 80, 80, 80, 255 } /* dark grey  */
#define COL_ENERGY_FALLBACK (Color) { 0, 200, 80, 255 } /* green      */
#define COL_SPEED_FALLBACK (Color) { 30, 144, 255, 255 } /* blue       */
#define COL_ODOM_FALLBACK (Color) { 255, 165, 0, 255 } /* orange     */
#define COL_ARROW_FALLBACK (Color) { 255, 255, 255, 255 } /* white      */

void RenderFrame(const Game* game, int screenW, int screenH)
{
    BeginDrawing();

    DrawLayerBackground(game, screenW, screenH); /* layer 0 */
    DrawLayerObstacle(game); /* layer 1 */
    DrawLayerCar(game); /* layer 2 */
    DrawLayerHUD(game, screenH); /* layer 3 */

    if (game->state == STATE_GAMEOVER)
        DrawLayerGameOver(game, screenW, screenH); /* layer 4 */

    EndDrawing();
}

void DrawLayerBackground(const Game* game, int screenW, int screenH)
{
    DrawBackground(&game->background, screenW, screenH);
}

void DrawLayerObstacle(const Game* game)
{
    const Obstacle* obs = &game->obstacle;
    const SpriteSheet* sheet = &obs->sheet;

    if (sheet->loaded) {
        DrawSprite(sheet, obs->frame, obs->pos.x, obs->pos.y);
    } else {
        DrawSpriteFallback(sheet, obs->frame,
            obs->pos.x, obs->pos.y,
            COL_OBSTACLE_FALLBACK);
    }
}

void DrawLayerCar(const Game* game)
{
    const Car* car = &game->car;
    const SpriteSheet* sheet = &car->sheet;

    if (sheet->loaded) {
        DrawSprite(sheet, car->frame, car->pos.x, car->pos.y);
    } else {
        DrawSpriteFallback(sheet, car->frame,
            car->pos.x, car->pos.y,
            COL_CAR_FALLBACK);
    }
}

void DrawLayerHUD(const Game* game, int screenH)
{
    const Car* car = &game->car;
    const HUD* hud = &game->hud;

    {
        const SpriteSheet* s = &hud->energy;
        int frame = car->energy;
        if (frame >= 0 && frame < s->count) {
            if (s->loaded)
                DrawSprite(s, frame, 10.0f, 10.0f);
            else
                DrawSpriteFallback(s, frame, 10.0f, 10.0f, COL_ENERGY_FALLBACK);
        }
    }

    {
        const SpriteSheet* s = &hud->speedometer;
        int frame = (int)car->velocity;
        if (frame >= 0 && frame < s->count) {
            if (s->loaded)
                DrawSprite(s, frame, 50.0f, 10.0f);
            else
                DrawSpriteFallback(s, frame, 50.0f, 10.0f, COL_SPEED_FALLBACK);
        }
    }

    {
        const SpriteSheet* s = &hud->odometer;
        int frame = car->distance;
        if (frame >= 0 && frame < s->count) {
            if (s->loaded)
                DrawSprite(s, frame, 98.0f, 90.0f);
            else
                DrawSpriteFallback(s, frame, 98.0f, 90.0f, COL_ODOM_FALLBACK);
        }
    }

    {
        const SpriteSheet* s = &hud->arrow;
        int frame = hud->arrowFrame;
        int arrowH = s->frames ? s->frames[0].height : 64;
        float y = (float)(screenH - arrowH);

        if (frame >= 0 && frame < s->count) {
            if (s->loaded)
                DrawSprite(s, frame, 10.0f, y);
            else
                DrawSpriteFallback(s, frame, 10.0f, y, COL_ARROW_FALLBACK);
        }
    }
}

void DrawLayerGameOver(const Game* game, int screenW, int screenH)
{
    if (game->gameOverLoaded) {
        float x = (float)(screenW - game->gameOverTexture.width) * 0.5f;
        float y = (float)(screenH - game->gameOverTexture.height) * 0.5f;
        DrawTexture(game->gameOverTexture, (int)x, (int)y, WHITE);
    } else {
        int pw = 360, ph = 120;
        int px = (screenW - pw) / 2;
        int py = (screenH - ph) / 2;

        DrawRectangle(px, py, pw, ph, (Color) { 0, 0, 0, 200 });
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
