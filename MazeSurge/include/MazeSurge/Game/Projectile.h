#pragma once
#include "MazeSurge/Core/Core.h"

struct Projectile
{
    bool active;
    float scale;
    float speed;
    float lifetime;
    float maxLifetime;
    XMFLOAT3 position;
    XMFLOAT3 direction;
    BBOX bbox;

    Projectile();
};