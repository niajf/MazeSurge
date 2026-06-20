#include "MazeSurge/Core/Core.h"
#include "MazeSurge/Game/Enemy.h"

Enemy::Enemy()
{
    active = false;
    scale = ENEMY_CELL_SCALE;
    speed = ENEMY_MOVE_SPEED;
    position = {0.f, 0.f, 0.f};
    bbox.setBBOX(position, scale);
}