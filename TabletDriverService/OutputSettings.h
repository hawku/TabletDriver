#pragma once

#include "Vector2D.h"

class OutputSettings {
public:

	// PixelPosition
	typedef struct {
		int x;
		int y;
	} PixelPosition;

	struct {
		PixelPosition pixelPosition;
		Vector2D lastPosition;
		Vector2D targetPosition;
	} relativeState;

	double relativeSensitivity;
	double relativeResetDistance;

	void ResetRelativeState(double x, double y);

	OutputSettings();
	~OutputSettings();
};

