#include "raylib.h"

#include <stddef.h>

#include "background.h"
#include "types.h"

void InitScroll(ScrollBackground* bg)
{
    bg->scrollOffset = 0.0f;
}

void UpdateScroll(ScrollBackground* bg, float speed)
{
    bg->scrollOffset += speed;

    float wrapAt = bg->loaded ? bg->texture.height : SCREEN_WIDTH;

    if (bg->scrollOffset >= wrapAt) {
        bg->scrollOffset -= wrapAt;
    }
}

void DrawBackground(const ScrollBackground* bg, int screenW, int screenH)
{
    if (!bg->loaded) {
        DrawRectangle(0, 0, screenW, screenH, DARKGRAY);

        int laneX = screenW / 2;
        int dashH = 60;
        int gapH = 40;
        int period = dashH + gapH;
        int offset = (int)bg->scrollOffset % period;

        for (int y = -period + offset; y < screenH + period; y += period) {
            DrawRectangle(laneX - 3, y, 6, dashH, YELLOW);
        }

        return;
    }

    float scaleX = (float)screenW / (float)bg->texture.width;
    float scaleY = (float)screenH / (float)bg->texture.height;

    float scale = (scaleX > scaleY) ? scaleX : scaleY;

    float texH = (float)bg->texture.height * scale;

    float yA = bg->scrollOffset - texH;
    float yB = bg->scrollOffset;

    DrawTextureEx(bg->texture, (Vector2) { 0.0f, yA }, 0.0f, scale, WHITE);
    DrawTextureEx(bg->texture, (Vector2) { 0.0f, yB }, 0.0f, scale, WHITE);
}
