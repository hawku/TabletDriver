#pragma once

#include "Vector2D.h"
#include "TabletState.h"
#include "Runnable.h"

class TabletMeasurement : public Runnable {
public:
	Vector2D minimum;
	Vector2D maximum;
	Vector2D points[10];
	int pointCount;

	int totalReports;
	int reportCounter;

	TabletState lastState;
	DWORD lastPointTime;


	TabletMeasurement();
	~TabletMeasurement();
	void Start();
	void Start(int reportCount);
	void Stop();
	void Update(TabletState state);
};

