#pragma once
#include "MazeSurge/Core/Core.h"
#include "MazeSurge/Utility/Collision.h"
#include "MazeSurge/Game/Enemy.h"
#include "MazeSurge/Game/Player.h"
#include "MazeSurge/Game/Dungeon.h"
#include "MazeSurge/Game/ProjectilePool.h"
#include <cstdlib>
#include <cmath>

// ============================================================
// EnemyManager — 敵のスポーン・更新・描画を一括管理する
// ============================================================
// オブジェクトプールパターンを使い、ゲーム中に new/delete を行わない。
// スポーン間隔は経過時間に応じて短縮され、難易度が徐々に上がる。
class EnemyManager
{
public:
    // プールを m_poolSize 分確保し、タイマーを初期化する。
    void Init();
    // プール内の非アクティブスロットを選び、迷路外周の円上にスポーンさせる。
    // プールが満杯なら何もしない（上限を超えた生成は行わない）。
    void SpawnEnemy(Dungeon &dungeon);
    // 毎フレーム呼ぶ。スポーンタイマー更新・敵の移動・プレイヤー/弾との衝突判定を処理する。
    void Update(float deltaTime, Player &player, Dungeon &dungeon, ProjectilePool &projectilePool);
    // アクティブな敵をすべて描画する。
    void Draw(Renderer &renderer) const;

private:
    size_t m_poolSize; // プールの最大サイズ（= 同時に存在できる敵の上限）
    // m_spawnTimer: 前回スポーンからの経過時間。m_spawnInterval を超えたらスポーンする。
    float m_spawnTimer;
    // m_elapsedTime: ゲーム開始からの累積時間。スポーン間隔の短縮計算に使用する。
    float m_elapsedTime;
    // m_spawnInterval: 現在のスポーン間隔（秒）。毎フレーム m_elapsedTime に基づき更新される。
    float m_spawnInterval;
    std::vector<Enemy> m_pool; // 敵のオブジェクトプール（固定サイズ）
};
