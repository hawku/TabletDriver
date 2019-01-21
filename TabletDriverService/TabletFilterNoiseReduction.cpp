#include "precompiled.h"
#include "TabletFilterNoiseReduction.h"


#define LOG_MODULE "Noise"
#include "Logger.h"


//
// Constructor
//
TabletFilterNoiseReduction::TabletFilterNoiseReduction() {
	distanceThreshold = 0;
	distanceMaximum = 1;
	iterations = 10;
	reportRate = 1;
	timeBegin = std::chrono::high_resolution_clock::now();
	timeLastReport = timeBegin;
	timeNow = timeBegin;
	outputPosition = &outputState.position;
}

//
// Destructor
//
TabletFilterNoiseReduction::~TabletFilterNoiseReduction() {
}

//
// TabletFilter methods
//

// Set target position
void TabletFilterNoiseReduction::SetTarget(TabletState *tabletState) {
	latestTarget.Set(tabletState->position);
	buffer.Add(tabletState->position);
	timeNow = tabletState->time;
	memcpy(&outputState, tabletState, sizeof(TabletState));
}

// Update
void TabletFilterNoiseReduction::Update() {

	// Report rate calculation
	double timeDelta = (timeNow - timeLastReport).count() / 1000000.0;
	if(timeDelta >= 1 && timeDelta <= 10) {
		reportRate += ((1000.0 / timeDelta) - reportRate) * (timeDelta / 1000.0) * 5.0;
	}
	timeLastReport = timeNow;

	// Velocity calculation
	double velocity = latestTarget.Distance(oldTarget) * reportRate;
	oldTarget.Set(latestTarget);

	// One position in the buffer?
	if(buffer.count == 1) {
		outputPosition->Set(latestTarget);
		return;
	}

	// Calculate geometric median from the buffer positions
	GetGeometricMedianVector(outputPosition, iterations);

	// Reset the buffer when distance to latest target position is larger than the threshold
	if(buffer.isValid) {

		// Distance between latest position and ring buffer geometric median
		double distance = latestTarget.Distance(outputPosition);

		// Distance larger than threshold -> modify the ring buffer
		if(distance > distanceThreshold && distanceMaximum > 0) {

			// Ratio between current distance and maximum distance
			double distanceRatio;

			// Skip distance ratio calculation
			if(distanceThreshold >= distanceMaximum) {
				distanceRatio = 1;

			// Distance ratio should be between 0.0 and 1.0
			// 0.0 -> distance == distanceThreshold
			// 1.0 -> distance == distanceMaximum
			} else {
				distanceRatio = (distance - distanceThreshold) / (distanceMaximum - distanceThreshold);
			}

			// Distance larger than maximum -> fill buffer with the latest target position
			if(distanceRatio >= 1.0) {
				for(int i = 0; i < buffer.count; i++) {
					buffer[i]->Set(latestTarget);
				}
				outputPosition->Set(latestTarget);

			// Move buffer positions and current position towards the latest target using linear interpolation
			// Amount of movement is the distance ratio between threshold and maximum
			} else {
				buffer.LerpAdd(latestTarget, distanceRatio);
				outputPosition->LerpAdd(latestTarget, distanceRatio);
			}

			// Debug message
			if(logger.IsDebugOutputEnabled()) {
				LOG_DEBUG("Threshold! D=%0.2f mm, R=%0.2f, V=%0.2f mm/s, V2=%0.2f mm/s\n",
					distance,
					distanceRatio,
					velocity, // True velocity
					distance * reportRate // Distance to velocity
				);
			}

		}
	}

	// Debug message
	if(logger.IsDebugOutputEnabled()) {
		double distance = outputPosition->Distance(latestTarget);
		double latency;
		if(velocity <= 0) {
			latency = 0;
		} else {
			latency = distance / velocity * 1000.0;
		}
		LOG_DEBUG("T=%0.0f B=%d Ta=[%0.2f,%0.2f] Po=[%0.2f,%0.2f] D=%0.2f R=%0.2f V=%0.2f L=%0.2f\n",
			(timeNow - timeBegin).count() / 1000000.0,
			buffer.count,
			latestTarget.x, latestTarget.y,
			outputPosition->x, outputPosition->y,
			distance,
			reportRate,
			velocity,
			latency
		);
	}


}

//
// Average Position Vector
//
bool TabletFilterNoiseReduction::GetAverageVector(Vector2D *output) {
	double x, y;
	if(!buffer.isValid) return false;

	x = y = 0;
	for(int i = 0; i < buffer.count; i++) {
		x += buffer[i]->x;
		y += buffer[i]->y;
	}
	output->x = x / buffer.count;
	output->y = y / buffer.count;
	return true;
}

//
// Geometric Median Position Vector
//
bool TabletFilterNoiseReduction::GetGeometricMedianVector(Vector2D *output, int iterations) {

	Vector2D candidate, next;
	double minimumDistance = 0.001;

	double denominator, dx, dy, distance, weight;
	int i;

	// Calculate the starting position
	if(GetAverageVector(&candidate)) {
	} else {
		return false;
	}

	// Iterate
	for(int iteration = 0; iteration < iterations; iteration++) {

		denominator = 0;

		// Loop through the buffer and calculate a denominator.
		for(i = 0; i < buffer.count; i++) {
			dx = candidate.x - buffer[i]->x;
			dy = candidate.y - buffer[i]->y;
			distance = sqrt(dx*dx + dy * dy);
			if(distance > minimumDistance) {
				denominator += 1.0 / distance;
			} else {
				denominator += 1.0 / minimumDistance;
			}
		}

		// Reset the next vector
		next.x = 0;
		next.y = 0;

		// Loop through the buffer and calculate a weighted average
		for(i = 0; i < buffer.count; i++) {
			dx = candidate.x - buffer[i]->x;
			dy = candidate.y - buffer[i]->y;
			distance = sqrt(dx*dx + dy * dy);
			if(distance > minimumDistance) {
				weight = 1.0 / distance;
			} else {
				weight = 1.0 / minimumDistance;
			}

			next.x += buffer[i]->x * weight / denominator;
			next.y += buffer[i]->y * weight / denominator;
		}

		// Set the new candidate vector
		candidate.x = next.x;
		candidate.y = next.y;
	}

	// Set output
	output->x = candidate.x;
	output->y = candidate.y;

	return true;

}

