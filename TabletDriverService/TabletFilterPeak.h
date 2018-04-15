#pragma once

#include "PositionRingBuffer.h"
#include "Vector2D.h"

class TabletFilterPeak : public TabletFilter {
public:

	PositionRingBuffer buffer;
	Vector2D position;
	double distanceThreshold;

	void SetTarget(Vector2D targetVector);
	void SetPosition(Vector2D vector);
	bool GetPosition(Vector2D *outputVector);
	void Update();


	TabletFilterPeak();
	~TabletFilterPeak();
};

