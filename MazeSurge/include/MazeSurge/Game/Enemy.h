#pragma once
#include "MazeSurge/Core/Common.h"
#include "MazeSurge/Core/Types.h"

struct Enemy
{
    bool active = false;
    XMFLOAT3 position;
    BBOX bbox;
    float scale = 0.8f;
    float speed = 2.5f;
    float damageTimer = 0.0f; // ダメージ被弾時の点滅用
};