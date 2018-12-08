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
		Vector2D targetPosition;
		Vector2D lastPosition;
		chrono::high_resolution_clock::time_point lastTime;
	} relativeState;

	double relativeSensitivity;
	double relativeResetDistance;
	double relativeResetTime;

	void ResetRelativeState(double x, double y, chrono::high_resolution_clock::time_point time);

	OutputSettings();
	~OutputSettings();
};

