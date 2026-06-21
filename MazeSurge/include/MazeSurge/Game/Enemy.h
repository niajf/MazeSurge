#pragma once
#include "MazeSurge/Core/Core.h"

struct Enemy
{
    XMFLOAT4 color;
    XMFLOAT3 position;
    BBOX bbox;
    bool active;
    float scale;
    float speed;

    Enemy();
};