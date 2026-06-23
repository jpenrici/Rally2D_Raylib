#include "raylib.h"

#include "collision.h"

#include <stdbool.h>
#include <stdlib.h>

static inline int CarWidth(const Car* car)
{
    return car->sheet.frames ? car->sheet.frames[0].width : 128;
}

static inline int CarHeight(const Car* car)
{
    return car->sheet.frames ? car->sheet.frames[0].height : 256;
}

static inline int ObstWidth(const Obstacle* o)
{
    return o->sheet.frames ? o->sheet.frames[0].width : 192;
}

static inline int ObstHeight(const Obstacle* o)
{
    return o->sheet.frames ? o->sheet.frames[0].height : 192;
}

int CheckCenterPoint(const Car* car, const Obstacle* obstacle)
{
    float cx = obstacle->pos.x + (float)ObstWidth(obstacle) * 0.5f;
    float cy = obstacle->pos.y + (float)ObstHeight(obstacle) * 0.5f;

    if (cy <= car->pos.y || cy >= car->pos.y + (float)CarHeight(car))
        return false;
    if (cx <= car->pos.x || cx >= car->pos.x + (float)CarWidth(car))
        return false;

    return true;
}

int CheckAABB(const Car* car, const Obstacle* obstacle)
{
    float carL = car->pos.x;
    float carR = car->pos.x + (float)CarWidth(car);
    float carT = car->pos.y;
    float carB = car->pos.y + (float)CarHeight(car);

    float obsL = obstacle->pos.x;
    float obsR = obstacle->pos.x + (float)ObstWidth(obstacle);
    float obsT = obstacle->pos.y;
    float obsB = obstacle->pos.y + (float)ObstHeight(obstacle);

    if (obsR <= carL || obsL >= carR)
        return false;
    if (obsB <= carT || obsT >= carB)
        return false;

    return true;
}

int ObstacleExited(const Obstacle* obstacle, int screenH)
{
    return obstacle->pos.y > (float)(screenH + ObstHeight(obstacle));
}

void ResetObstacle(Obstacle* obstacle, int screenW)
{
    int ow = ObstWidth(obstacle);
    int oh = ObstHeight(obstacle);

    obstacle->pos.y = -(float)(oh / 2);

    int rangeW = screenW - 2 * ow;
    if (rangeW < 0)
        rangeW = 0;
    obstacle->pos.x = (float)(GetRandomValue(0, rangeW) + ow);

    int maxFrame = obstacle->sheet.count > 1 ? obstacle->sheet.count - 2 : 0;
    obstacle->frame = GetRandomValue(0, maxFrame);
}

void ResolveCollision(Car* car, Obstacle* obstacle, int screenW)
{
    car->energy--;

    car->velocity -= 5.0f;
    if (car->velocity < 0.0f)
        car->velocity = 0.0f;

    car->step = car->velocity / 4.0f;

    TraceLog(LOG_INFO, "Collision — energy: %d  velocity: %.0f",
        car->energy, (double)car->velocity);

    ResetObstacle(obstacle, screenW);
}
