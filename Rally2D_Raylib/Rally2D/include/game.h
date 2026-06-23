#pragma once

#include "types.h"

void GameInit(Game* game, int screenW, int screenH);
void GameHandleInput(Game* game);
void GameUpdate(Game* game);
void GameRender(const Game* game);
void GameQuit(Game* game);
