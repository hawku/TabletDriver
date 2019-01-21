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
		bool firstPosition;
		PixelPosition pixelPosition;
		Vector2D targetPosition;
		Vector2D lastPosition;
		std::chrono::high_resolution_clock::time_point lastTime;
	} relativeState;

	Vector2D relativeSensitivity;
	double relativeResetDistance;
	double relativeResetTime;
	bool relativeDragMove;

	void ResetRelativeState(double x, double y, std::chrono::high_resolution_clock::time_point time);

	OutputSettings();
	~OutputSettings();
};

