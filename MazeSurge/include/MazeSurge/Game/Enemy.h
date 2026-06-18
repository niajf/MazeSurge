#include "MazeSurge/Core/Common.h"

struct Enemy
{
    bool active = false;
    XMFLOAT3 position;
    int hp = 30;
    float speed = 2.5f;
    float damageTimer = 0.0f; // ダメージ被弾時の点滅用
};