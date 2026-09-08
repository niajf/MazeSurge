#include "MazeSurge/Game/ProjectilePool.h"

void ProjectilePool::Init()
{
    m_poolSize = PROJECTILEPOOL_POOL_SIZE;
    m_elapsedTime = 0.f;
    m_fireInterval = PROJECTILEPOOL_FIRE_INTERVAL_TIME;

    // プールサイズ分の Projectile を事前確保するオブジェクトプールパターン。
    // 発射のたびに new/delete を行わないことでメモリ断片化を防ぐ。
    for (size_t i = 0; i < m_poolSize; i++)
        m_pool.emplace_back();
}

Projectile *ProjectilePool::Get()
{
    // プールを先頭から線形探索し、最初の非アクティブスロットを返す。
    for (size_t i = 0; i < m_pool.size(); i++)
    {
        if (!m_pool[i].active)
            return &m_pool[i];
    }

    // プールが満杯なら新たに生成しない（nullptr を返して呼び出し側で無視）。
    return nullptr;
}

void ProjectilePool::Update(float deltaTime, const Camera &camera, const Dungeon &dungeon, const Player &player, const InputState &inputState)
{
    m_elapsedTime += deltaTime;

    // ---- 発射処理 ----
    // lMouseDown（押しっぱなし）かつ発射間隔を超えていれば弾を生成する。
    // m_elapsedTime をリセットすることで連射レートを制御する。
    if (inputState.lMouseDown && m_elapsedTime > m_fireInterval)
    {
        m_elapsedTime = 0.0f;

        // プールに非アクティブな弾を取り出す。
        Projectile *proj = Get();

        if (proj)
        {
            // planeY=-0.5f はプレイヤー中心(Y≈0)より半ユニット下の水平面。
            // 弾の発射高さに合わせることで弾道がクリック位置と視覚的に一致する。
            XMFLOAT3 hitPos = camera.ScreenToWorldOnPlane(inputState.mouseX, inputState.mouseY, -0.5f);
            XMFLOAT3 playerPos = player.GetPosition();

            // プレイヤー → クリック位置 への正規化方向ベクトルを発射方向にする。
            XMVECTOR dir_norm = XMVector3Normalize(
                XMVectorSubtract(XMLoadFloat3(&hitPos), XMLoadFloat3(&playerPos)));
            XMFLOAT3 dir_float3_norm;
            XMStoreFloat3(&dir_float3_norm, dir_norm);

            // プレイヤーの速さベクトルを計算
            XMFLOAT3 playerVelVec = player.GetVelocityVector();
            float playerSpeed = player.GetSpeed();
            playerVelVec.x *= playerSpeed;
            playerVelVec.z *= playerSpeed;

            // 弾の速さベクトルを計算
            dir_float3_norm.x *= proj->speed;
            dir_float3_norm.z *= proj->speed;

            // 弾にプレイヤーの慣性が乗るようにする
            dir_float3_norm.z += playerVelVec.z;
            dir_float3_norm.x += playerVelVec.x;

            proj->active = true;
            proj->position = playerPos;
            proj->bbox.setBBOX(proj->position, proj->scale);
            proj->direction = dir_float3_norm;
            proj->lifetime = 0.f; // 発射時にライフタイムをリセット。
        }
    }

    // ---- 弾丸の移動とライフタイム管理 ----
    for (size_t i = 0; i < m_pool.size(); i++)
    {
        if (!m_pool[i].active)
            continue;

        m_pool[i].lifetime += deltaTime;
        // ライフタイム超過で弾を非アクティブ化（画面外まで飛び続けるのを防ぐ）。
        if (m_pool[i].lifetime > m_pool[i].maxLifetime)
        {
            m_pool[i].active = false;
            continue;
        }

        // 一定方向への等速直線移動。direction は発射時に正規化済み。
        m_pool[i].position.x += deltaTime * m_pool[i].direction.x;
        // m_pool[i].position.y += deltaTime * m_pool[i].speed * m_pool[i].direction.y;
        m_pool[i].position.z += deltaTime * m_pool[i].direction.z;

        // 移動後に bbox を再計算して衝突判定を正確にする。
        m_pool[i].bbox.setBBOX(m_pool[i].position, m_pool[i].scale);
    }
};

void ProjectilePool::Draw(Renderer &renderer) const
{
    for (size_t i = 0; i < m_pool.size(); i++)
    {
        if (!m_pool[i].active)
            continue;

        // +0.5f は床面（Y=0）より半ユニット上に描画するためのオフセット。
        // ピボットが底面中心のキューブを床面上に浮かせる。
        XMMATRIX world = XMMatrixScaling(m_pool[i].scale, m_pool[i].scale, m_pool[i].scale) * XMMatrixTranslation(m_pool[i].position.x,
                                                                                                                  m_pool[i].position.y + 0.5f,
                                                                                                                  m_pool[i].position.z);
        renderer.DrawCube(world, m_pool[i].color);
    }
};

bool ProjectilePool::DeactiveOnCollision(const BBOX &targetBbox)
{
    // アクティブな弾丸を全検索し、対象 bbox と重なった最初の弾丸を非アクティブ化する。
    // 呼び出し元（EnemyManager）は戻り値が true なら敵も非アクティブ化する。
    // 1 フレームで複数弾が同一敵に当たる場合、先頭の 1 発のみ処理される。
    for (size_t i = 0; i < m_pool.size(); i++)
    {
        if (!m_pool[i].active)
            continue;

        if (Collision::checkAABB(m_pool[i].bbox, targetBbox))
        {
            m_pool[i].active = false;
            return true; // 衝突あり（敵を倒す）。
        }
    }

    return false; // 衝突なし。
}
