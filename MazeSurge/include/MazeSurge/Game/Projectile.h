#pragma once
#include "MazeSurge/Core/Core.h"

struct Projectile
{
    XMFLOAT4 color;
    XMFLOAT3 position;
    XMFLOAT3 direction;
    BBOX bbox;
    bool active;
    float scale;
    float speed;
    float lifetime;
    float maxLifetime;

    Projectile();
};