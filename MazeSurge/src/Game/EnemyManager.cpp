#include "MazeSurge/Game/EnemyManager.h"
#include "MazeSurge/Audio/SoundManager.h"

void EnemyManager::Init()
{
    m_poolSize = ENEMYMANAGER_POOL_SIZE;
    m_spawnTimer = 0.f;
    m_elapsedTime = 0.f;
    m_spawnInterval = ENEMYMANAGER_SPAWN_INTERVAL;

    // プールサイズ分の Enemy を事前確保する（オブジェクトプールパターン）。
    // ゲーム中の new/delete を排除してメモリ断片化を防ぐ。
    for (size_t i = 0; i < m_poolSize; i++)
        m_pool.emplace_back();
}

void EnemyManager::SpawnEnemy(Dungeon &dungeon, Player &player)
{
    // プールを先頭から線形探索し、最初の非アクティブスロットを再利用する。
    for (size_t i = 0; i < m_pool.size(); i++)
    {
        if (!m_pool[i].active)
        {

            m_pool[i].active = true;

            // プレイヤーから一定距離離れた床セルに敵をポップさせる
            // 初め、ランダムな角度の方向に床セルがあるかを調べ、
            // 床セルでない場合は、その角度を始点として角度を10度ずつ変化させ、探索を継続
            bool isEnemyPlaced = false;
            int degree = rand() % 360;
            int startDegree = degree;

            while (!isEnemyPlaced)
            {
                // 迷路外周の円上（半径 = グリッドサイズ × セルサイズ）のランダムな角度にスポーンさせる。
                // これにより迷路の外側から敵が侵入する演出になる。
                float radian = XM_PI * degree / 180.f;
                float norm = 5.f * dungeon.getCellSize();
                m_pool[i].position.x = norm * std::cos(radian) + player.GetPosition().x;
                m_pool[i].position.y = 0.f; // 敵は常に床面（Y=0）に配置する。
                m_pool[i].position.z = norm * std::sin(radian) + player.GetPosition().z;

                if (dungeon.IsFloor(m_pool[i].position))
                {
                    // 敵をポップできたら探索を中止
                    isEnemyPlaced = true;
                }

                else
                {
                    // 角度を10度ずつ更新
                    degree += 10;
                    degree % 360;
                }

                // どの角度にも敵をポップさせられない場合、探索を中止
                if (degree = startDegree)
                {
                    break;
                }
            }

            // 敵の座標を補正
            int gridX, gridZ;
            dungeon.WorldToGrid(m_pool[i].position.x, m_pool[i].position.x, gridX, gridZ);
            XMFLOAT3 adjPos = dungeon.GridToWorld(gridX, gridZ);
            m_pool[i].position.x = adjPos.x;
            m_pool[i].position.z = adjPos.z;
            m_pool[i].bbox.setBBOX(m_pool[i].position, m_pool[i].scale);

            return; // 1 体スポーンしたら即リターン（1 回の呼び出しで 1 体のみ）。
        }
    }
    // プールが満杯の場合は無視（上限を超えて生成しない）。
}

void EnemyManager::Update(float deltaTime, Player &player, Dungeon &dungeon, ProjectilePool &projectilePool)
{
    m_elapsedTime += deltaTime;

    // スポーン間隔を経過時間に応じて短縮する（難易度の漸進的上昇）。
    // 式: interval = BASE / (1 + t / SCALE)
    // t=0s: BASE(3.0s), t=60s: BASE/2, t=300s: BASE/6 と双曲線的に加速する。
    // 分母が 1.0f から始まるためゼロ割りは発生せず、間隔が 0 になることもない。
    m_spawnInterval = ENEMYMANAGER_SPAWN_INTERVAL / (1.0f + m_elapsedTime / ENEMYMANAGER_SPAWN_TIME_SCALE);

    m_spawnTimer += deltaTime;
    if (m_spawnTimer >= m_spawnInterval)
    {
        m_spawnTimer = 0;
        SpawnEnemy(dungeon, player);
    }

    std::vector<int> dz = {-1, 0, 1, 0};
    std::vector<int> dx = {0, 1, 0, -1};

    for (size_t i = 0; i < m_pool.size(); i++)
    {
        if (!m_pool[i].active)
            continue;

        // ---- プレイヤーへの追尾移動 ----
        // 敵からプレイヤーへの方向ベクトルを正規化して速度に掛ける。
        // Y 成分も計算に含まれるが、スポーン時に Y=0 に固定しているため影響は小さい。
        XMFLOAT3 dirFloatNorm = GetMoveVector(dungeon, player, m_pool[i].position);

        m_pool[i].position.x += deltaTime * m_pool[i].speed * dirFloatNorm.x;
        m_pool[i].position.y += deltaTime * m_pool[i].speed * dirFloatNorm.y;
        m_pool[i].position.z += deltaTime * m_pool[i].speed * dirFloatNorm.z;

        // 移動後に bbox を更新して衝突判定を正確にする。
        m_pool[i].bbox.setBBOX(m_pool[i].position, m_pool[i].scale);

        // ---- プレイヤーとの衝突判定 ----
        // 衝突したら敵を非アクティブ化し、プレイヤーの HP を 1 減らす。
        if (Collision::checkAABB(player.GetBBOX(), m_pool[i].bbox))
        {
            m_pool[i].active = false;
            player.hitEnemy();
        }

        // ---- 弾丸との衝突判定 ----
        // DeactiveOnCollision は弾が当たった場合に弾も非アクティブ化して true を返す。
        if (projectilePool.DeactiveOnCollision(m_pool[i].bbox))
        {
            m_pool[i].active = false;
            SoundManager::GetInstance().PlayHitEnemySE();
        }
    }
}

void EnemyManager::Draw(Renderer &renderer) const
{
    for (size_t i = 0; i < m_pool.size(); i++)
    {
        if (!m_pool[i].active)
            continue;

        // Y に ENEMY_CELL_SCALE * 0.5f を加算して床面から浮かせる（ピボットが底面中心のため）。
        XMMATRIX world = XMMatrixScaling(m_pool[i].scale, m_pool[i].scale, m_pool[i].scale) * XMMatrixTranslation(m_pool[i].position.x,
                                                                                                                  m_pool[i].position.y + ENEMY_CELL_SCALE * 0.5f,
                                                                                                                  m_pool[i].position.z);
        renderer.DrawCube(world, m_pool[i].color);
    }
}

XMFLOAT3 EnemyManager::GetMoveVector(const Dungeon &dungeon, const Player &player, XMFLOAT3 pos)
{
    int enemyGridX, enemyGridZ;
    dungeon.WorldToGrid(pos.x, pos.z, enemyGridX, enemyGridZ);

    int playerGridX, playerGridZ;
    dungeon.WorldToGrid(player.GetPosition().x, player.GetPosition().z, playerGridX, playerGridZ);

    int gridSize = dungeon.getMazeSize();

    // プレイヤーと敵が同じセルにいる場合、ワールド座標から直接移動ベクトルを作成
    if (playerGridX == enemyGridX && playerGridZ == enemyGridZ)
    {
        XMFLOAT3 fPlayer = player.GetPosition();
        XMVECTOR vPlayer = XMLoadFloat3(&fPlayer);
        XMVECTOR vEnemy = XMLoadFloat3(&pos);
        XMVECTOR vDir = XMVectorSubtract(vPlayer, vEnemy);
        XMVECTOR vDirNorm = XMVector3Normalize(vDir);
        XMFLOAT3 result;
        XMStoreFloat3(&result, vDirNorm);

        return result;
    }

    // 自身の4近傍セルの内、プレイヤーまでの距離が最も近いセルの方向へ移動する
    std::vector<int> dz = {-1, 0, 1, 0};
    std::vector<int> dx = {0, 1, 0, -1};

    int minCost = 1000000;
    int ansX = -1, ansZ = -1;
    for (int i = 0; i < 4; i++)
    {
        int nz = enemyGridZ + dz[i];
        int nx = enemyGridX + dx[i];

        if (!(0 <= nz && nz < gridSize && 0 <= nx && nx < gridSize))
            continue;

        int cost = player.GetMoveCostToPlayer(nx, nz);

        // 到達不可能なセル
        if (cost == -1)
            continue;

        if (cost < minCost)
        {
            minCost = cost;
            ansZ = nz;
            ansX = nx;
        }
    }

    // 移動可能なセルが4近傍に存在しない場合、移動しない
    if (ansX == -1 && ansZ == -1)
    {
        return XMFLOAT3(0.f, 0.f, 0.f);
    }

    XMFLOAT3 fTo = dungeon.GridToWorld(ansX, ansZ);
    XMVECTOR vTo = XMLoadFloat3(&fTo);
    XMVECTOR vFrom = XMLoadFloat3(&pos);
    XMVECTOR vDir = XMVectorSubtract(vTo, vFrom);
    XMVECTOR vDirNorm = XMVector3Normalize(vDir);

    XMFLOAT3 result;
    XMStoreFloat3(&result, vDirNorm);

    return result;
}
