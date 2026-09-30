#include "MazeSurge/Game/Projectile.h"

Projectile::Projectile()
{
    // プール生成時は非アクティブ状態ProjectilePool::Get() で active=true にする
    active = false;
    scale = PROJECTILE_CELL_SCALE;
    speed = PROJECTILE_MOVE_SPEED;

    // lifetime は発射時に 0 にリセットされるここでの初期化は念のため
    lifetime = 0.f;
    maxLifetime = PROJECTILE_LIFE_LIMIT_TIME;
    color = PROJECTILE_CELL_COLOR;

    // 発射前の仮位置・方向active=false の間は参照されない
    position = {0.f, 0.f, 0.f};
    direction = {0.f, 0.f, 0.f};
    
    // bbox は Get() 呼び出し時に position/scale を基に再計算される
    bbox.setBBOX(position, scale);
}
