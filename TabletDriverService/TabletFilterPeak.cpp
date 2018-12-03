#include "stdafx.h"
#include "TabletFilterPeak.h"

#define LOG_MODULE "Peak"
#include "Logger.h"

//
// Constructor
//
TabletFilterPeak::TabletFilterPeak() {

	// Buffer length
	buffer.SetLength(3);

	// Default distance threshold
	distanceThreshold = 10;
}


//
// Destructor
//
TabletFilterPeak::~TabletFilterPeak() {
}


//
// TabletFilter methods
//
// Set target position
void TabletFilterPeak::SetTarget(TabletState *tabletState) {
	buffer.Add(tabletState->position);
}
// Set position
void TabletFilterPeak::SetPosition(Vector2D vector) {
	position.x = vector.x;
	position.y = vector.y;
}
// Get position
bool TabletFilterPeak::GetPosition(Vector2D *outputVector) {
	outputVector->x = position.x;
	outputVector->y = position.y;
	return true;
}
// Update
void TabletFilterPeak::Update() {
	Vector2D oldPosition;
	double distance;

	// Buffer valid?
	if(
		buffer.GetLatest(&oldPosition, -1)
		&&
		buffer.GetLatest(&position, 0)
	) {

		// Jump longer than the distance threshold?
		distance = oldPosition.Distance(position);
		if(distance > distanceThreshold) {

			/*
			LOG_DEBUG("PEAK! %0.2f,%0.2f -> %0.2f,%0.2f = %0.2f mm\n",
				oldPosition.x, oldPosition.y,
				position.x, position.y,
				distance
			);
			*/

			position.x = oldPosition.x;
			position.y = oldPosition.y;

			// Reset buffer
			buffer.Reset();

		}
	}
}