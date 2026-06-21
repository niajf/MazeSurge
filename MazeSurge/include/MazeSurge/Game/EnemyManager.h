#pragma once
#include "MazeSurge/Core/Core.h"
#include "MazeSurge/Utility/Collision.h"
#include "MazeSurge/Game/Enemy.h"
#include "MazeSurge/Game/Player.h"
#include "MazeSurge/Game/Dungeon.h"
#include "MazeSurge/Game/ProjectilePool.h"
#include <cstdlib>
#include <cmath>

class EnemyManager
{
public:
    void Init();                                                                                    // プールを確保し各パラメータを初期値に設定する
    void SpawnEnemy(Dungeon &dungeon);                                                              // プールから非アクティブな敵を取得してランダム位置に出現させる
    void Update(float deltaTime, Player &player, Dungeon &dungeon, ProjectilePool &projectilePool); // 敵の移動・衝突・スポーンタイマーを毎フレーム処理する
    void Draw(Renderer &renderer) const;                                                            // アクティブな敵を全て描画する

private:
    size_t m_poolSize;         // プールに確保する敵の最大数
    float m_spawnTimer;        // 敵生成に用いるタイマー（敵を生成してからの時間）
    float m_elapsedTime;       // 敵の生成間隔を調整に用いるタイマー（ゲームが開始してからの時間）
    float m_spawnInterval;     // 敵の生成間隔
    std::vector<Enemy> m_pool; // 敵の管理に用いるプール
};