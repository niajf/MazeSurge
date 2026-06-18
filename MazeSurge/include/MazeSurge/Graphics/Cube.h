#pragma once

#include "MazeSurge/Core/Common.h"

class Cube
{
public:
	XMFLOAT3 position;
	XMFLOAT3 scale;
	float angle;

	Cube(XMFLOAT3 position, XMFLOAT3 scale, float angle) : position(position), scale(scale), angle(angle) {}
};