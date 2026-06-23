#pragma once

#include "types.h"

#define ASSET_BACKGROUND "assets/background.png"
#define ASSET_GAME_OVER "assets/gameOver.png"
#define ASSET_ARROW "assets/arrow-sprite.png"
#define ASSET_CAR "assets/player-sprite.png"
#define ASSET_ENERGY "assets/energy-sprite.png"
#define ASSET_OBSTACLE "assets/obstacle-sprite.png"
#define ASSET_ODOMETER "assets/odometer-sprite.png"
#define ASSET_SPEEDOMETER "assets/speedometer-sprite.png"

void LoadAllAssets(Game* game, int screenW, int screenH);
void UnloadAllAssets(Game* game);
