#pragma once
#include "MazeSurge/Core/Common.h"
#include "MazeSurge/Core/Types.h"

class Collision
{
public:
    static bool checkAABB(const BBOX &box1, const BBOX &box2);
};