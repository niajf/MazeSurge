#include "MazeSurge/Game/Dungeon.h"

void Dungeon::GenerateTestMap()
{
	// デバッグ用の固定マップ。Generate() を呼ばずに手動でグリッドを構築する。
	m_grid.push_back({START, WALL, WALL, FLOOR, WALL});
	m_grid.push_back({FLOOR, FLOOR, FLOOR, FLOOR, WALL});
	m_grid.push_back({FLOOR, FLOOR, WALL, WALL, WALL});
	m_grid.push_back({FLOOR, CHECKPOINT, FLOOR, FLOOR, WALL});
	m_grid.push_back({FLOOR, WALL, WALL, FLOOR, GOAL});
}

void Dungeon::Init()
{
	m_wallColor = DUNGEON_WALL_COLOR;
	m_goalColor = DUNGEON_GOAL_COLOR;
	m_checlPointColor = DUNGEON_CP_COLOR;
	m_mazeSize = DUNGEON_GRID_SIZE;
	m_numCheckPoint = DUNGEON_CHECKPOINT_NUM;
	m_wallScale = DUNGEON_WALL_SCALE;
	m_goalScale = DUNGEON_GOAL_SCALE;
	m_checkPointScale = DUNGEON_CP_SCALE;

	Generate();
}

void Dungeon::Generate()
{
	// 時刻シードで毎回異なる迷路を生成する。
	std::srand(static_cast<unsigned int>(std::time(nullptr)));

	// まず全マスを WALL で初期化し、穴掘り法で通路を掘っていく。
	m_grid.resize(m_mazeSize, std::vector<CellType>(m_mazeSize, WALL));

	// 穴掘りの起点。奇数座標から始めることで壁の間に 1 マスの通路が確保される。
	std::pair<int, int> startPos = {1, 1};

	// DFS（深さ優先探索）スタックを使った再帰的穴掘り法（Recursive Backtracker）。
	// スタックに座標を積み、掘れる隣接マスがなくなったらバックトラックする。
	std::stack<std::pair<int, int>> stk;
	stk.push(startPos);
	m_grid[startPos.first][startPos.second] = CellType::START;

	// 上・右・下・左の 4 方向の差分。2 マス飛ばして進むため、間の壁も掘る。
	std::vector<int> di = {-1, 0, 1, 0};
	std::vector<int> dj = {0, 1, 0, -1};

	// ランダムな開始インデックス (0〜3) から 4 要素を連続アクセスするために
	// {0,1,2,3} を 2 回繰り返した配列を用意し、モジュロ計算を省略している。
	std::vector<int> order = {0, 1, 2, 3, 0, 1, 2, 3};

	// ---- 穴掘り法メインループ ----
	while (!stk.empty())
	{
		auto [nowI, nowJ] = stk.top();

		// 4 方向をランダムな順序で試す。
		int startIndex = rand() % 4;
		bool all = true; // 全方向が行き止まりかどうかのフラグ
		for (int k = 0; k < 4; k++)
		{
			// 2 マス先の座標（間の壁を跨いで掘る）。
			int newI = nowI + di[order[startIndex + k]] * 2;
			int newJ = nowJ + dj[order[startIndex + k]] * 2;

			// グリッド範囲外はスキップ。
			if (!(0 <= newI && newI < m_mazeSize && 0 <= newJ && newJ < m_mazeSize))
				continue;

			// 未訪問（WALL のまま）のマスなら掘り進める。
			if (m_grid[newI][newJ] == CellType::WALL)
			{
				// 中間の壁マスも FLOOR にする。
				m_grid[nowI + di[order[startIndex + k]]][nowJ + dj[order[startIndex + k]]] = CellType::FLOOR;
				m_grid[nowI + di[order[startIndex + k]] * 2][nowJ + dj[order[startIndex + k]] * 2] = CellType::FLOOR;
				stk.push({newI, newJ});
				all = false;
				break; // 1 方向掘ったら再びスタックトップから探索する。
			}
		}

		// 全方向が壁または訪問済みならバックトラック。
		if (all)
			stk.pop();
	}

	// ---- BFS でスタートから各マスへのマンハッタン距離を計測 ----
	// 目的: 最遠マスをゴールに、行き止まりをチェックポイント候補にする。
	std::vector<std::vector<int>> distL1(m_mazeSize, std::vector<int>(m_mazeSize, -1));

	// 通路の端（出口が 1 方向しかない）を行き止まりとして記録する。
	std::vector<std::pair<int, int>> deadEnds;

	// BFS キュー: (行, 列, スタートからの距離)
	std::queue<std::tuple<int, int, int>> que;

	// スタートから最も遠いセルをゴールにするための追跡変数。
	std::pair<int, int> goalGrid = startPos;
	int maxDist = -1;

	que.push({startPos.first, startPos.second, 0});

	while (!que.empty())
	{
		auto [nowI, nowJ, nowDist] = que.front();
		que.pop();

		// 未訪問マスのみ処理（distL1==-1 が未訪問を示す）。
		if (distL1[nowI][nowJ] == -1)
		{
			distL1[nowI][nowJ] = nowDist;
			// より遠いマスが見つかったらゴール候補を更新。
			if (maxDist < nowDist)
			{
				maxDist = nowDist;
				goalGrid = {nowI, nowJ};
			}
		}
		else
			continue; // 訪問済みはスキップ。

		// 隣接する通路マスをキューに追加し、行き止まり（通路が 1 方向のみ）を検出する。
		int freeDirCount = 0;
		for (int k = 0; k < 4; k++)
		{
			int newI = nowI + di[k];
			int newJ = nowJ + dj[k];

			if (!(0 <= newI && newI < m_mazeSize && 0 <= newJ && newJ < m_mazeSize))
				continue;

			if (m_grid[newI][newJ] == CellType::FLOOR)
			{
				freeDirCount++;
				que.push({newI, newJ, nowDist + 1});
			}
		}

		// 通路が 1 方向しかない = 行き止まり（チェックポイント候補）。
		if (freeDirCount == 1)
			deadEnds.push_back({nowI, nowJ});
	}

	// BFS 訪問順ではスタート(1,1)が最初に deadEnds に入る（通路 1 本しかない）。
	// 先頭 2 要素を除くことで、スタート直近へのチェックポイント配置を防ぐ。
	deadEnds.erase(deadEnds.begin(), deadEnds.begin() + 2);

	// チェックポイント数が行き止まり数を超えないようにクランプする。
	// -1 はゴールに使う行き止まり分を除くため。
	m_numCheckPoint = std::min(m_numCheckPoint, deadEnds.size() - 1);

	// ---- チェックポイントをランダムな行き止まりに配置 ----
	// Fisher-Yates シャッフルの簡易版: インデックス配列から random で選んで削除。
	std::vector<size_t> idxVector(m_numCheckPoint);
	std::iota(idxVector.begin(), idxVector.end(), 0);
	for (size_t i = 0; i < m_numCheckPoint; i++)
	{
		size_t idx = rand() % idxVector.size();
		size_t idxDeadEnd = idxVector[idx];
		m_grid[deadEnds[idxDeadEnd].first][deadEnds[idxDeadEnd].second] = CellType::CHECKPOINT;
		idxVector.erase(idxVector.begin() + idx); // 選んだインデックスを除外して重複を防ぐ。
	}

	// BFS で検出した最遠マスをゴールにする。
	m_grid[goalGrid.first][goalGrid.second] = CellType::GOAL;
}

void Dungeon::Draw(Renderer &renderer) const
{
	for (int i = 0; i < m_mazeSize; i++)
	{
		for (int j = 0; j < m_mazeSize; j++)
		{
			// FLOOR と START は描画しない（床面のみ表示）。
			if (m_grid[i][j] == WALL)
			{
				// i=行(Z方向), j=列(X方向) で GridToWorld を呼ぶ。
				XMFLOAT3 pos = GridToWorld(j, i);
				XMMATRIX world = XMMatrixScaling(m_wallScale, m_wallScale, m_wallScale) * XMMatrixTranslation(pos.x, pos.y, pos.z);
				renderer.DrawCube(world, m_wallColor);
			}
			else if (m_grid[i][j] == GOAL)
			{
				XMFLOAT3 pos = GridToWorld(j, i);
				XMMATRIX world = XMMatrixScaling(m_goalScale, m_goalScale, m_goalScale) * XMMatrixTranslation(pos.x, pos.y, pos.z);
				renderer.DrawCube(world, m_goalColor);
			}
			else if (m_grid[i][j] == CHECKPOINT)
			{
				XMFLOAT3 pos = GridToWorld(j, i);
				XMMATRIX world = XMMatrixScaling(m_checkPointScale, m_checkPointScale, m_checkPointScale) * XMMatrixTranslation(pos.x, pos.y, pos.z);
				renderer.DrawCube(world, m_checlPointColor);
			}
		}
	}
}

bool Dungeon::IsWall(float worldX, float worldZ) const
{
	// ワールド座標をグリッド座標に変換してからセル判定に委譲する。
	int gridX = 0, gridZ = 0;
	WorldToGrid(worldX, worldZ, gridX, gridZ);
	return IsWallCell(gridX, gridZ);
}

bool Dungeon::IsWallCell(int gridX, int gridZ) const
{
	// グリッド範囲外は壁として扱う（迷路の外周に出られないようにする）。
	if (gridX < 0 || gridX >= m_mazeSize || gridZ < 0 || gridZ >= m_mazeSize)
		return true;

	// m_grid は行優先（row-major）: outer=行=Z、inner=列=X。
	// m_grid[gridZ][gridX] の順であることに注意。
	return m_grid[gridZ][gridX] == WALL;
}

XMFLOAT3 Dungeon::GridToWorld(int gridX, int gridZ) const
{
	// グリッド中心 (m_mazeSize-1)/2 を原点(0,0)に対応させる線形変換。
	// 例: mazeSize=11 の場合、グリッド(5,5) → ワールド(0,0)。
	return {
		m_wallScale * (float)(gridX - (m_mazeSize - 1) / 2),
		0.0f,
		m_wallScale * (float)(gridZ - (m_mazeSize - 1) / 2)};
}

void Dungeon::WorldToGrid(float worldX, float worldZ, int &gridX, int &gridZ) const
{
	// GridToWorld の逆変換:
	//   worldX = m_wallScale * (gridX - (m_mazeSize-1)/2)
	//   → gridX = worldX / m_wallScale + (m_mazeSize-1)/2
	// std::round で最近傍グリッドに丸め、clamp で範囲外アクセスを防ぐ。
	float gx = worldX / m_wallScale + (float)(m_mazeSize - 1) / 2.0f;
	float gz = worldZ / m_wallScale + (float)(m_mazeSize - 1) / 2.0f;
	gridX = std::clamp((int)std::round(gx), 0, m_mazeSize - 1);
	gridZ = std::clamp((int)std::round(gz), 0, m_mazeSize - 1);
}

bool Dungeon::IsCheckPoint(XMFLOAT3 playerPos)
{
	int gridX = 0, gridZ = 0;
	WorldToGrid(playerPos.x, playerPos.z, gridX, gridZ);

	if (m_grid[gridZ][gridX] == CellType::CHECKPOINT)
	{
		// チェックポイントを踏んだら FLOOR に書き換えて再取得を防ぐ。
		// Draw() でも描画されなくなり、視覚的にも消える。
		m_grid[gridZ][gridX] = CellType::FLOOR;
		return true;
	}

	return false;
}

bool Dungeon::IsGoal(XMFLOAT3 playerPos)
{
	int gridX = 0, gridZ = 0;
	WorldToGrid(playerPos.x, playerPos.z, gridX, gridZ);

	// ゴールセルは踏んでも書き換えない（ゲームクリア判定のみ）。
	if (m_grid[gridZ][gridX] == CellType::GOAL)
		return true;

	return false;
}

XMFLOAT3 Dungeon::GetStartPosition() const
{
	// グリッドを線形探索して START セルを見つけ、ワールド座標に変換して返す。
	// Generate() で必ず (1,1) が START になるため、最悪でも 2 回目の反復で見つかる。
	for (int i = 0; i < m_mazeSize; i++)
	{
		for (int j = 0; j < m_mazeSize; j++)
		{
			if (m_grid[i][j] == START)
				return GridToWorld(j, i); // j=列(X), i=行(Z)
		}
	}

	// START が見つからない場合（通常は発生しない）の安全な初期位置。
	return {0.0f, 0.0f, 0.0f};
}

XMFLOAT3 Dungeon::GetGoalPosition() const
{
	for (int i = 0; i < m_mazeSize; i++)
	{
		for (int j = 0; j < m_mazeSize; j++)
		{
			if (m_grid[i][j] == GOAL)
				return GridToWorld(j, i);
		}
	}
	return {0.0f, 0.0f, 0.0f};
}
