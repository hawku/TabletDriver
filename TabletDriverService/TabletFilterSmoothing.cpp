#include "stdafx.h"
#include "TabletFilterSmoothing.h"

//
// Constructor
//
TabletFilterSmoothing::TabletFilterSmoothing() {
	latency = 2.0;
	weight = 1.000;
	threshold = 0.9;
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
void TabletFilterSmoothing::SetTarget(Vector2D vector) {
	this->target.x = vector.x;
	this->target.y = vector.y;
}

// Set current position
void TabletFilterSmoothing::SetPosition(Vector2D vector) {
	position.x = vector.x;
	position.y = vector.y;
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

	deltaX = target.x - position.x;
	deltaY = target.y - position.y;
	distance = sqrt(deltaX*deltaX + deltaY * deltaY);

	// Distance large enough?
	if(distance > 0.01) {
		position.x += deltaX * weight;
		position.y += deltaY * weight;

		// Too small distance -> set output values as target values
	} else {
		position.x = target.x;
		position.y = target.y;
	}

}





// Set position
double TabletFilterSmoothing::SetPosition(double x, double y) {
	this->position.x = x;
	this->position.y = y;
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


