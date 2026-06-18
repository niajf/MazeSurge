#pragma once
#include "MazeSurge/Common.h"

struct Projectile
{
    bool active = false;
    XMFLOAT3 position;
    XMFLOAT3 direction;
    float velocity = 5.0f;
    float lifetime = 0.0f;
    float maxLifetime = 10.0f;
};