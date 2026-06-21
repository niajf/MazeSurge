#pragma once
#include "MazeSurge/Core/Core.h"
#include "MazeSurge/Game/Dungeon.h"
#include "MazeSurge/Graphics/Renderer.h"
#include <algorithm>

class Player
{
public:
	void Init(const XMFLOAT3 &staticPosition);
	void Update(float deltaTime, Dungeon &g_dungeon, const InputState &inputState);
	void Draw(Renderer &renderer) const;
	void hitEnemy();

	XMFLOAT3 GetPosition() const { return m_position; };
	BBOX GetBBOX() const { return m_bbox; };
	int GetHP() const { return m_hp; };

private:
	// プレイヤーの移動方向を計算
	XMFLOAT3 CalcMoveVelocity() const;

	// キーの入力状況を保持する変数
	bool m_keyW = false;
	bool m_keyA = false;
	bool m_keyS = false;
	bool m_keyD = false;

	XMFLOAT4 m_color;	 // プレイヤーブロックの色
	XMFLOAT3 m_position; // プレイヤーの座標
	BBOX m_bbox;		 // プレイヤーのBoundingBox
	float m_scale;		 // プレイヤーブロックのスケール
	float m_speed;		 // プレイヤーの移動速度
	int m_hp;			 // プレイヤーのHP
};