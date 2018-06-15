#include "stdafx.h"
#include "TabletBenchmark.h"

//
// Constructor
//
TabletBenchmark::TabletBenchmark() {
	maxX = 0;
	minX = 0;
	maxY = 0;
	minY = 0;
	totalPackets = 0;
	packetCounter = 0;
}

//
// Destructor
//
TabletBenchmark::~TabletBenchmark() {
}


//
// Start tablet benchmark
//
void TabletBenchmark::Start(int packetCount) {
	maxX = -10000;
	maxY = -10000;
	minX = 10000;
	minY = 10000;
	totalPackets = packetCount;
	packetCounter = packetCount;
	isRunning = true;
}

void TabletBenchmark::Update(Vector2D position) {
	if (isRunning) {
		if (packetCounter > 0) {
			if (position.x < minX) minX = position.x;
			if (position.x > maxX) maxX = position.x;
			if (position.y < minY) minY = position.y;
			if (position.y > maxY) maxY = position.y;
			packetCounter--;
		}
		else {
			isRunning = false;
		}
	}
}