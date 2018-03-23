#include "stdafx.h"
#include "TabletFilterNoiseReduction.h"


//
// Constructor
//
TabletFilterNoiseReduction::TabletFilterNoiseReduction() {
	bufferMaxLength = sizeof(buffer) / sizeof(Vector2D);
	bufferLength = 0;
	bufferPositionCount = 0;
	bufferCurrentIndex = 0;
	distanceThreshold = 0;
	iterations = 10;
}

//
// Destructor
//
TabletFilterNoiseReduction::~TabletFilterNoiseReduction() {
}


//
// Set buffer length
//
void TabletFilterNoiseReduction::SetBufferLength(int length) {
	if(length > bufferMaxLength) {
		bufferLength = bufferMaxLength;
	} else {
		bufferLength = length;
	}
}


//
// TabletFilter methods
//

// Set target position
void TabletFilterNoiseReduction::SetTarget(Vector2D targetVector) {
	if(isValid) {
		double distance = lastTarget.Distance(targetVector);
		if(distance > distanceThreshold) {
			ResetBuffer();
		}
	}
	lastTarget.Set(targetVector);
	AddBuffer(targetVector);
}

// Get current position
void TabletFilterNoiseReduction::SetPosition(Vector2D vector) {
	position.x = vector.x;
	position.y = vector.y;
}

// Get current position
bool TabletFilterNoiseReduction::GetPosition(Vector2D *outputVector) {
	outputVector->x = position.x;
	outputVector->y = position.y;
	return true;
}

// Update
void TabletFilterNoiseReduction::Update() {
	GetGeometricMedianVector(&position, iterations);
}



void TabletFilterNoiseReduction::ResetBuffer() {
	bufferPositionCount = 0;
	bufferCurrentIndex = 0;
	isValid = false;
}

//
// Add position to buffer
//
void TabletFilterNoiseReduction::AddBuffer(Vector2D vector) {
	buffer[bufferCurrentIndex].x = vector.x;
	buffer[bufferCurrentIndex].y = vector.y;
	bufferCurrentIndex++;
	bufferPositionCount++;
	if(bufferPositionCount > bufferLength) {
		bufferPositionCount = bufferLength;
	}
	if(bufferCurrentIndex >= bufferLength) {
		bufferCurrentIndex = 0;
	}
	isValid = true;
}

//
// Average Position Vector
//
bool TabletFilterNoiseReduction::GetAverageVector(Vector2D *output) {
	double x, y;
	if(!isValid) return false;

	x = y = 0;
	for(int i = 0; i < bufferPositionCount; i++) {
		x += buffer[i].x;
		y += buffer[i].y;
	}
	output->x = x / bufferPositionCount;
	output->y = y / bufferPositionCount;
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
		for(i = 0; i < bufferPositionCount; i++) {
			dx = candidate.x - buffer[i].x;
			dy = candidate.y - buffer[i].y;
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
		for(i = 0; i < bufferPositionCount; i++) {
			dx = candidate.x - buffer[i].x;
			dy = candidate.y - buffer[i].y;
			distance = sqrt(dx*dx + dy * dy);
			if(distance > minimumDistance) {
				weight = 1.0 / distance;
			} else {
				weight = 1.0 / minimumDistance;
			}

			next.x += buffer[i].x * weight / denominator;
			next.y += buffer[i].y * weight / denominator;
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

