#pragma once
#include "MazeSurge/Core/Common.h"
#include "MazeSurge/Core/Types.h"

struct Projectile
{
    bool active = false;
    XMFLOAT3 position;
    XMFLOAT3 direction;
    BBOX bbox;
    float scale = 0.2f;
    float velocity = 5.0f;
    float lifetime = 0.0f;
    float maxLifetime = 10.0f;
};