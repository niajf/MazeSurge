#include "MazeSurge/Game/Projectile.h"

Projectile::Projectile()
{
    active = false;
    scale = PROJECTILE_CELL_SCALE;
    speed = PROJECTILE_MOVE_SPEED;
    lifetime = 0.f;
    maxLifetime = PROJECTILE_LIFE_LIMIT_TIME;
    position = {0.f, 0.f, 0.f};
    direction = {0.f, 0.f, 0.f};
    bbox.setBBOX(position, scale);
}