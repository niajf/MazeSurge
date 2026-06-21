#pragma once
#include "MazeSurge/Core/Core.h"

class Collision
{
public:
    // 2つの AABB が重なっているかを返す（XZ 平面上の 2D 判定）
    static bool checkAABB(const BBOX &box1, const BBOX &box2);
};