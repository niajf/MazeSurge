#pragma once
#include "MazeSurge/Core/Common.h"
#include "MazeSurge/Core/Types.h"
#include "MazeSurge/Game/Dungeon.h"
#include "MazeSurge/Graphics/Renderer.h"
#include <algorithm>

class Player
{
public:
	void Init(const XMFLOAT3 &staticPosition);

	void Update(float deltaTime, Dungeon &g_dungeon);

	// プレイヤーキューブの描画
	void Draw() const;

	XMFLOAT3 GetPosition() const { return m_position; };
	BBOX getBBOX() const { return m_bbox; };
	int GetHP() const { return m_hp; };

	bool keyW = false;
	bool keyA = false;
	bool keyS = false;
	bool keyD = false;

private:
	XMFLOAT3 m_position = {0.0f, 0.0f, 0.0f};
	BBOX m_bbox;
	float m_scale = 0.8f;
	float m_speed = 6.0f;
	int m_hp = 100;

	// プレイヤーの移動方向を計算
	XMFLOAT3 CalcMoveVelocity() const;
};

extern Player g_player;