#include "raylib.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "sprite.h"

void PrepareSheet(SpriteSheet* sheet, const char* path, int rows, int cols,
    int frameW, int frameH)
{
    if (sheet == NULL)
        return;

    int count = rows * cols;

    sheet->frames = (Sprite*)malloc((size_t)count * sizeof(Sprite));
    sheet->count = count;

    if (sheet->frames == NULL) {
        fprintf(stderr, "[sprite] malloc failed for %s\n", path);
        sheet->loaded = false;
        return;
    }

    int idx = 0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            Sprite* s = &sheet->frames[idx++];
            s->srcX = j * frameW; // column offset in pixels
            s->srcY = i * frameH; // row    offset in pixels
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

void DrawSprite(const SpriteSheet* sheet, int frame, float x, float y)
{
    if (sheet == NULL)
        return;

    if (frame < 0 || frame >= sheet->count)
        return;

    if (!sheet->loaded) {
        return;
    }

    const Sprite* s = &sheet->frames[frame];

    Rectangle src = { (float)s->srcX, (float)s->srcY, (float)s->srcW,
        (float)s->srcH };

    Rectangle dst = { (float)(int)x, (float)(int)y, (float)s->width,
        (float)s->height };

    Vector2 origin = { 0.0f, 0.0f };

    DrawTexturePro(sheet->texture, src, dst, origin, 0.0f, WHITE);
}

void DrawSpriteFallback(const SpriteSheet* sheet, int frame, float x, float y,
    Color color)
{
    if (sheet == NULL)
        return;

    if (frame < 0 || frame >= sheet->count)
        return;

    const Sprite* s = &sheet->frames[frame];

    DrawRectangle((int)x, (int)y, s->width, s->height, color);
    DrawRectangleLines((int)x, (int)y, s->width, s->height, BLACK);
}

void AnimateSprite(int* frame, int* timer, int firstFrame, int lastFrame,
    int ticksPerFrame)
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

void ResizeSprite(SpriteSheet* sheet, int newW, int newH)
{
    if (sheet == NULL || sheet->frames == NULL)
        return;

    for (int i = 0; i < sheet->count; i++) {
        if (newW > 0)
            sheet->frames[i].width = newW;
        if (newH > 0)
            sheet->frames[i].height = newH;
    }
}

void UnloadSheet(SpriteSheet* sheet)
{
    if (sheet == NULL)
        return;

    free(sheet->frames);
    sheet->frames = NULL;
    sheet->count = 0;

    if (sheet->loaded) {
        UnloadTexture(sheet->texture);
        sheet->loaded = false;
    }
}
