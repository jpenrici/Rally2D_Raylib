#pragma once

#include "types.h"

int CheckCenterPoint(const Car* car, const Obstacle* obstacle);
int CheckAABB(const Car* car, const Obstacle* obstacle);
int ObstacleExited(const Obstacle* obstacle, int screenH);
void ResetObstacle(Obstacle* obstacle, int screenW);
void ResolveCollision(Car* car, Obstacle* obstacle, int screenW);
