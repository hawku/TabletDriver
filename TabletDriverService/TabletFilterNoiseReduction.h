#pragma once

#include "PositionRingBuffer.h"

class TabletFilterNoiseReduction : public TabletFilter {
public:

	PositionRingBuffer buffer;
	Vector2D position;
	Vector2D lastTarget;

	int iterations;
	double distanceThreshold;

	void SetTarget(Vector2D targetVector, double h);
	void SetPosition(Vector2D vector, double h);
	bool GetPosition(Vector2D *outputVector);
	void Update();

	bool GetAverageVector(Vector2D *output);
	bool GetGeometricMedianVector(Vector2D *output, int iterations);

	TabletFilterNoiseReduction();
	~TabletFilterNoiseReduction();
};

