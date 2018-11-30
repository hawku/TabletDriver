#pragma once
#include "TabletFilter.h"
#include "PositionRingBuffer.h"

#include <chrono>


class TabletFilterAntiSmoothing : public TabletFilter {
public:
	PositionRingBuffer buffer;
	Vector2D position;
	Vector2D latestTarget;
	Vector2D lastTarget;

	double power;
	double multiplier;

	double reportRate;
	double velocity;
	double acceleration;
	double jerk;
	double lastVelocity;
	double lastAcceleration;
	double lastJerk;



	chrono::high_resolution_clock::time_point timeBegin;
	chrono::high_resolution_clock::time_point timeLastReport;
	chrono::high_resolution_clock::time_point timeNow;

	void SetTarget(Vector2D targetVector);
	void SetPosition(Vector2D vector);
	bool GetPosition(Vector2D *outputVector);
	void Update();

	TabletFilterAntiSmoothing();
	~TabletFilterAntiSmoothing();
};

