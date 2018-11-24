#pragma once

#include "Vector2D.h"

class RelativeMouseState {
public:
	// PixelPosition
	typedef struct {
		int x;
		int y;
	} PixelPosition;

	PixelPosition pixelPosition;
	Vector2D lastPosition;
	Vector2D targetPosition;
	double sensitivity;
	double resetDistance;

	RelativeMouseState();
	~RelativeMouseState();
};

