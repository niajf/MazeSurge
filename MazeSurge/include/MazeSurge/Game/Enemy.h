#pragma once
#include "MazeSurge/Core/Core.h"

struct Enemy
{
    bool active;
    float scale;
    float speed;
    XMFLOAT3 position;
    BBOX bbox;

    Enemy();
};