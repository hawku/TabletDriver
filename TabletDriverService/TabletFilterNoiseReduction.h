#pragma once
#include "TabletFilter.h"
#include "PositionRingBuffer.h"

#include <chrono>


class TabletFilterNoiseReduction : public TabletFilter {
public:

	PositionRingBuffer buffer;
	Vector2D position;
	Vector2D latestTarget;
	Vector2D oldTarget;

	int iterations;
	double distanceThreshold;
	double distanceMaximum;
	double reportRate;
	chrono::high_resolution_clock::time_point timeBegin;
	chrono::high_resolution_clock::time_point timeLastReport;
	chrono::high_resolution_clock::time_point timeNow;


	void SetTarget(TabletState *tabletState);
	void SetPosition(Vector2D vector);
	bool GetPosition(Vector2D *outputVector);
	void Update();

	bool GetAverageVector(Vector2D *output);
	bool GetGeometricMedianVector(Vector2D *output, int iterations);

	TabletFilterNoiseReduction();
	~TabletFilterNoiseReduction();
};

