#pragma once
#include "MazeSurge/Core/Core.h"
#include "MazeSurge/Game/Projectile.h"
#include "MazeSurge/Game/Dungeon.h"
#include "MazeSurge/Game/Player.h"
#include "MazeSurge/Utility/Collision.h"

// ============================================================
// ProjectilePool — 弾のオブジェクトプール管理
// ============================================================
// 固定サイズの Projectile 配列をプールとして持ち、
// ゲーム中に new/delete を行わずに弾の生成・廃棄を行う。
// 発射処理（Update()内）では Camera::ScreenToWorldOnPlane() を使い、
// クリック位置をワールド座標に変換して発射方向を決定する。
class ProjectilePool
{
public:
    // プールを m_poolSize 分確保し、発射タイマーを初期化する。
    void Init();
    // プール内の非アクティブスロットを選び、位置と方向を設定してポインタを返す。
    // プールが満杯なら nullptr を返す（呼び出し側で無視する）。
    Projectile *Get();
    // 毎フレーム呼ぶ。左クリック時の発射処理・弾の移動・寿命管理を行う。
    // Camera は ScreenToWorldOnPlane() による照準計算に使用する。
    void Update(float deltaTime, const Camera &camera, const Dungeon &dungeon, const Player &player, const InputState &inputState);
    // アクティブな弾をすべて描画する。
    void Draw(Renderer &renderer) const;
    // アクティブな弾の中で targetBbox と衝突した最初の弾を非アクティブ化し、
    // 衝突があれば true を返す。EnemyManager が敵の死亡判定に使用する。
    bool DeactiveOnCollision(const BBOX &targetBbox);

private:
    std::vector<Projectile> m_pool; // 弾のオブジェクトプール（固定サイズ）
    size_t m_poolSize;              // プールの最大サイズ（= 同時に存在できる弾の上限）
    float m_fireInterval;           // 連射間隔（秒）: この時間が経過するまで発射しない
    // m_elapsedTime は最後に発射してからの経過時間。
    // m_fireInterval と比較して発射可否を判断する。
    float m_elapsedTime;
};
