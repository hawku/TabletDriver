#pragma once
#include "TabletFilter.h"
#include "PositionRingBuffer.h"

#include <chrono>


class TabletFilterNoiseReduction : public TabletFilter {
public:

	PositionRingBuffer buffer;
	Vector2D latestTarget;
	Vector2D oldTarget;
	Vector2D *outputPosition;

	int iterations;
	double distanceThreshold;
	double distanceMaximum;
	double reportRate;
	std::chrono::high_resolution_clock::time_point timeBegin;
	std::chrono::high_resolution_clock::time_point timeLastReport;
	std::chrono::high_resolution_clock::time_point timeNow;


	void SetTarget(TabletState *tabletState);
	void Update();

	bool GetAverageVector(Vector2D *output);
	bool GetGeometricMedianVector(Vector2D *output, int iterations);

	TabletFilterNoiseReduction();
	~TabletFilterNoiseReduction();
};

