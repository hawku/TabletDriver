#pragma once

#include "PositionRingBuffer.h"
#include "Vector2D.h"

class TabletFilterPeak : public TabletFilter {
public:

	PositionRingBuffer buffer;
	double distanceThreshold;

	void SetTarget(TabletState *tabletState);
	void Update();


	TabletFilterPeak();
	~TabletFilterPeak();
};