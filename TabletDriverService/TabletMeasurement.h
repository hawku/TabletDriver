#pragma once

#include "Vector2D.h"
#include "TabletState.h"

class TabletMeasurement {
public:
	Vector2D minimum;
	Vector2D maximum;
	Vector2D points[10];
	int pointCount;

	int totalPackets;
	int packetCounter;
	bool isRunning;

	TabletState lastState;
	DWORD lastPointTime;


	TabletMeasurement();
	~TabletMeasurement();
	void Start();
	void Start(int packetCount);
	void Stop();
	void Update(TabletState state);
};

