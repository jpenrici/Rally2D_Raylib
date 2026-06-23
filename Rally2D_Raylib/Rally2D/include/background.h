#pragma once

#include "types.h"

void InitScroll(ScrollBackground* bg);
void UpdateScroll(ScrollBackground* bg, float speed);
void DrawBackground(const ScrollBackground* bg, int screenW, int screenH);
