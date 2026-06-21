#include "MazeSurge/Core/Core.h"
#include "MazeSurge/Game/Enemy.h"

Enemy::Enemy()
{
    color = ENEMY_CELL_COLOR;
    // スポーン前の仮位置。active=false の間は描画・衝突判定の対象外なので値は何でもよい。
    position = {0.f, 0.f, 0.f};
    // bbox は position と scale から計算されるため、両者が確定した後に更新する。
    bbox.setBBOX(position, scale);
    // プール生成時は非アクティブ状態にしておき、SpawnEnemy() で active=true にする。
    active = false;
    scale = ENEMY_CELL_SCALE;
    speed = ENEMY_MOVE_SPEED;
}
