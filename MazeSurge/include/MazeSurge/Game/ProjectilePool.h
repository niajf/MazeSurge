#pragma once
#include "MazeSurge/Core/Core.h"
#include "MazeSurge/Game/Projectile.h"
#include "MazeSurge/Game/Dungeon.h"
#include "MazeSurge/Game/Player.h"
#include "MazeSurge/Utility/Collision.h"

class ProjectilePool
{
public:
    void Init();                                                                                                                    // プールを確保し各パラメータを初期値に設定する
    Projectile *Get(XMFLOAT3 playerPos, XMFLOAT3 dir);                                                                              // プールから非アクティブな弾を取り出して位置・方向を設定し返す
    void Update(float deltaTime, const Camera &camera, const Dungeon &dungeon, const Player &player, const InputState &inputState); // 発射・移動・寿命・衝突判定を毎フレーム処理する
    void Draw(Renderer &renderer) const;                                                                                            // アクティブな弾を全て描画する
    bool DeactiveOnCollision(const BBOX &targetBbox);                                                                               // 指定 BBOX と衝突した弾を非アクティブ化し、衝突したかを返す

private:
    std::vector<Projectile> m_pool; // 弾の管理に用いるプール（固定サイズ）
    size_t m_poolSize;              // プールに確保する弾の最大数
    float m_fireInterval;           // 連射間隔（秒）
    float m_elapsedTime;            // 最後に発射してからの経過時間
};
