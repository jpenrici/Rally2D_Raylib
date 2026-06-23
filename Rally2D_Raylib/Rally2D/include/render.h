#pragma once

#include "types.h"

void RenderFrame(const Game* game, int screenW, int screenH);
void DrawLayerBackground(const Game* game, int screenW, int screenH);
void DrawLayerObstacle(const Game* game);
void DrawLayerCar(const Game* game);
void DrawLayerHUD(const Game* game, int screenH);
void DrawLayerGameOver(const Game* game, int screenW, int screenH);
