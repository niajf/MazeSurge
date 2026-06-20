#pragma once
#include "MazeSurge/Core/Core.h"

class Collision
{
public:
    static bool checkAABB(const BBOX &box1, const BBOX &box2);
};