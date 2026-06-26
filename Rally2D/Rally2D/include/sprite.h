#pragma once

#include "types.h"

void PrepareSheet(SpriteSheet* sheet, const char* path, int rows, int cols, int frameW, int frameH);
void DrawSprite(const SpriteSheet* sheet, int frame, float x, float y);
void DrawSpriteFallback(const SpriteSheet* sheet, int frame, float x, float y, Color color);
void AnimateSprite(int* frame, int* timer, int firstFrame, int lastFrame, int ticksPerFrame);
void ResizeSprite(SpriteSheet* sheet, int newW, int newH);
void UnloadSheet(SpriteSheet* sheet);
