#pragma once

#include "Vector2D.h"

class TabletBenchmark {
public:
	double minX;
	double maxX;
	double minY;
	double maxY;
	int totalPackets;
	int packetCounter;
	bool isRunning;

	TabletBenchmark();
	~TabletBenchmark();
	void Start(int packetCount);
	void Update(Vector2D position);
};

