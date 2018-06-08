#include "stdafx.h"
#include "TabletFilterSmoothing.h"

//
// Constructor
//
TabletFilterSmoothing::TabletFilterSmoothing() {
	latency = 2.0;
	weight = 1.000;
	threshold = 0.9;
	antichatterType = 2;
	antichatterRange = 0.15;
	antichatterStrength = 3;
	antichatterOffset = 0.0;
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

	double deltaX, deltaY, distance, weightModifier;

	deltaX = target.x - position.x;
	deltaY = target.y - position.y;
	distance = sqrt(deltaX*deltaX + deltaY * deltaY);

	// Regular smoothing
	if (antichatterType == 0) { // it's not original smoothing because it works every time
		position.x += deltaX * weight;
		position.y += deltaY * weight;
	}
	// Devocub smoothing
	else {
		if (antichatterType == 1)
		{
			if (distance > antichatterRange) { // use regular smoothing when exceed distance
				position.x += deltaX * weight;
				position.y += deltaY * weight;
			}
			else { // 1st type of smoothing
				weightModifier = 1 / antichatterStrength;
				if (weightModifier > 1) weightModifier = 1;
				position.x += deltaX * (weight * weightModifier);
				position.y += deltaY * (weight * weightModifier);
			}
		}
		else
			if (antichatterType == 2) {
				if (distance > antichatterRange) { // use regular smoothing when exceed distance
					position.x += deltaX * weight;
					position.y += deltaY * weight;
				}
				else { // 2nd type of smoothing
					weightModifier = pow(distance, antichatterStrength*-1) + antichatterOffset;
					if (weightModifier < 1) weightModifier = 1;
					position.x += deltaX * (weight / weightModifier);
					position.y += deltaY * (weight / weightModifier);
				}
			}
		else
			if (antichatterType == 3) {
				if (distance > antichatterRange) { // disable filtering when exceed distance
					position.x = target.x;
					position.y = target.y;
				}
				else { // same as 2
					weightModifier = pow(distance, antichatterStrength*-1) + antichatterOffset;
					if (weightModifier < 1) weightModifier = 1;
					position.x += deltaX * (weight / weightModifier);
					position.y += deltaY * (weight / weightModifier);
				}
			}
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


