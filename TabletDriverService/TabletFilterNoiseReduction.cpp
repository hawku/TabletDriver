#include "stdafx.h"
#include "TabletFilterNoiseReduction.h"


#define LOG_MODULE "Noise"
#include "Logger.h"


//
// Constructor
//
TabletFilterNoiseReduction::TabletFilterNoiseReduction() {
	distanceThreshold = 0;
	iterations = 10;
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
void TabletFilterNoiseReduction::SetTarget(Vector2D targetVector) {
	latestTarget.Set(targetVector);
	buffer.Add(targetVector);
}

// Set position
void TabletFilterNoiseReduction::SetPosition(Vector2D vector) {
	position.x = vector.x;
	position.y = vector.y;
}

// Get position
bool TabletFilterNoiseReduction::GetPosition(Vector2D *outputVector) {
	outputVector->x = position.x;
	outputVector->y = position.y;
	return true;
}

// Update
void TabletFilterNoiseReduction::Update() {

	// One position in the buffer?
	if(buffer.count == 1) {
		position.x = buffer[0]->x;
		position.y = buffer[0]->y;
		return;
	}

	// Calculate geometric median from the buffer positions
	GetGeometricMedianVector(&position, iterations);

	// Reset the buffer when distance to latest target position is larger than the threshold
	if(buffer.isValid) {

		// Distance between latest position and ring buffer geometric median
		double distance = latestTarget.Distance(position);

		// Distance larger than threshold -> modify the ring buffer
		if(distance > distanceThreshold) {

			// Distance factor
			double distanceFactor = 1;
			if(distanceThreshold > 0) {
				distanceFactor = distance / distanceThreshold;
			}

			// Buffer fill count (distance 10 times larger than threshold -> fill whole buffer)
			int fillCount = (int)(distanceFactor * 0.1 * buffer.length);
			if(fillCount > buffer.length) fillCount = buffer.length;

			// Fill part of the ring buffer with the latest position
			for(int i = 0; i < fillCount; i++) {
				buffer.Add(latestTarget);
			}

			// Debug
			if(tablet->debugEnabled) {
				LOG_DEBUG("Noise distance threshold! D:%0.2f Fa:%0.2f Fi:%d\n", distance, distanceFactor, fillCount);
			}


			// Max fill -> set the position to latest target position
			if(fillCount >= buffer.length) {
				position.Set(latestTarget);

			// Calculate a new geometric median
			} else {
				GetGeometricMedianVector(&position, iterations);
			}

		}
	}

	// Debug
	if(tablet->debugEnabled) {
		LOG_DEBUG("Noise: B:%d T:%0.2f,%0.2f O:%0.2f,%0.2f D:%0.2f\n",
			buffer.count,
			latestTarget.x, latestTarget.y,
			position.x, position.y,
			position.Distance(latestTarget)
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

