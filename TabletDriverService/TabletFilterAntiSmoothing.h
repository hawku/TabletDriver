#pragma once
#include "TabletFilter.h"
#include "PositionRingBuffer.h"

#include <chrono>


class TabletFilterAntiSmoothing : public TabletFilter {
public:
	Vector2D position;
	Vector2D latestTarget;
	Vector2D oldTarget;

	double shape;
	double compensation;

	void SetTarget(TabletState *tabletState);
	void SetPosition(Vector2D vector);
	bool GetPosition(Vector2D *outputVector);
	void Update();

	TabletFilterAntiSmoothing();
	~TabletFilterAntiSmoothing();
private:
	double reportRate;
	double reportRateAverage;
	double velocity;
	double acceleration;
	double jerk;
	double oldVelocity;
	double oldAcceleration;
	double oldJerk;
	int ignoreInvalidReports;
	chrono::high_resolution_clock::time_point timeBegin;
	chrono::high_resolution_clock::time_point timeLastReport;
	chrono::high_resolution_clock::time_point timeNow;

};

