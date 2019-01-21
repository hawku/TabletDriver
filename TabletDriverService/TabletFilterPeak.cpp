#include "precompiled.h"
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
	memcpy(&outputState, tabletState, sizeof(TabletState));
}

//
// Update
//
void TabletFilterPeak::Update() {
	Vector2D oldPosition;
	double distance;

	// Buffer valid?
	if(
		buffer.GetLatest(&oldPosition, -1)
		&&
		buffer.GetLatest(&outputState.position, 0)
	) {

		// Jump longer than the distance threshold?
		distance = oldPosition.Distance(outputState.position);
		if(distance > distanceThreshold) {

			/*
			LOG_DEBUG("PEAK! %0.2f,%0.2f -> %0.2f,%0.2f = %0.2f mm\n",
				oldPosition.x, oldPosition.y,
				position.x, position.y,
				distance
			);
			*/

			outputState.position.Set(oldPosition);

			// Reset buffer
			buffer.Reset();

		}
	}
}