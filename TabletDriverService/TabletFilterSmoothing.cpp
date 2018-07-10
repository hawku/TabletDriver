#include "stdafx.h"
#include "TabletFilterSmoothing.h"

//
// Constructor
//
TabletFilterSmoothing::TabletFilterSmoothing() {
	latency = 2.0;
	weight = 1.000;
	threshold = 0.9;
	AntichatterEnabled = true;
	antichatterStrength = 3;
	antichatterMultiplier = 1;
	antichatterOffsetX = 0.0;
	antichatterOffsetY = 0.0;
}


//
// Destructor
//
TabletFilterSmoothing::~TabletFilterSmoothing() {
}


//
// TabletFilter methods
//

// Set target position
void TabletFilterSmoothing::SetTarget(Vector2D vector, double h) {
	this->target.x = vector.x;
	this->target.y = vector.y;
	this->z = h;
}

// Set current position
void TabletFilterSmoothing::SetPosition(Vector2D vector, double h) {
	position.x = vector.x;
	position.y = vector.y;
	z = h;
}

// Get current position
bool TabletFilterSmoothing::GetPosition(Vector2D *outputVector) {
	outputVector->x = position.x;
	outputVector->y = position.y;
	return true;
}

// Update
void TabletFilterSmoothing::Update() {

	double deltaX, deltaY, distance;
	double weightModifier; // long float?

	deltaX = target.x - position.x;
	deltaY = target.y - position.y;
	distance = sqrt(deltaX * deltaX + deltaY * deltaY);

	// Regular smoothing
	if (!AntichatterEnabled) { // it's not original smoothing because it works every time
		position.x += deltaX * weight;
		position.y += deltaY * weight;
	}
	// Devocub smoothing
	else {

		//if (antichatterOffsetY < 0) antichatterOffsetY = 0;

		// Increase weight of filter in {formula} times
		//weightModifier = pow((distance + antichatterOffsetX), antichatterStrength*-1)*antichatterMultiplier + antichatterOffsetY;

		weightModifier = pow((distance + antichatterOffsetX), antichatterStrength*-1)*antichatterMultiplier;

		// Limit minimum
		if (weightModifier + antichatterOffsetY < 0)
			weightModifier = 0;
		else
			weightModifier = pow((distance + antichatterOffsetX), antichatterStrength*-1)*antichatterMultiplier + antichatterOffsetY;

		// Limit maximum (for non wacom tablets)
		/*if (weightModifier > 1000)
			weightModifier = 1000;*/

		weightModifier = weight / weightModifier;
		if (weightModifier > 1) weightModifier = 1;
		else if (weightModifier < 0) weightModifier = 0;

		position.x += deltaX * weightModifier;
		position.y += deltaY * weightModifier;

		// z ~= height, strength of signal from pen
		// 480 z is 14-41
		// 470 z is 0-30
	}
}





// Set position
double TabletFilterSmoothing::SetPosition(double x, double y, double h) {
	this->position.x = x;
	this->position.y = y;
	this->z = h;
}


//
// Calculate filter latency
//
double TabletFilterSmoothing::GetLatency(double filterWeight, double interval, double threshold) {
	double target = 1 - threshold;
	double stepCount = -log(1 / target) / log(1 - filterWeight);
	return stepCount * interval;
}
double TabletFilterSmoothing::GetLatency(double filterWeight) {
	return this->GetLatency(filterWeight, timerInterval, threshold);
}
double TabletFilterSmoothing::GetLatency() {
	return this->GetLatency(weight, timerInterval, threshold);
}


//
// Calculate filter weight
//
double TabletFilterSmoothing::GetWeight(double latency, double interval, double threshold) {
	double stepCount = latency / interval;
	double target = 1 - threshold;
	return 1 - 1 / pow(1 / target, 1 / stepCount);
}
double TabletFilterSmoothing::GetWeight(double latency) {
	return this->GetWeight(latency, this->timerInterval, this->threshold);
}

// Set Latency
void TabletFilterSmoothing::SetLatency(double latency) {
	this->weight = GetWeight(latency);
	this->latency = latency;
}


