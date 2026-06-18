#include "MazeSurge/Utility/Collision.h"

// AABB衝突判定
bool Collision::checkAABB(const BBOX &box1, const BBOX &box2)
{
    if (box1.minX <= box2.maxX && box1.maxX >= box2.minX &&
        box1.minZ <= box2.maxZ && box1.maxZ >= box2.minZ)
    {
        return true;
    }

    return false;
}