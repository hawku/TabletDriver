#include "stdafx.h"
#include "TabletFilterSmoothing.h"

//
// Constructor
//
TabletFilterSmoothing::TabletFilterSmoothing() {
	latency = 2.0;
	latency2 = 2.0;
	weight = 1.000;
	threshold = 0.9;
	antichatterType = 2;
	antichatterRange = 0.15;
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

	double deltaX, deltaY, distance, weightModifier, heightModifier;

	deltaX = target.x - position.x;
	deltaY = target.y - position.y;
	distance = sqrt(deltaX*deltaX + deltaY*deltaY);

	// Regular smoothing
	if (antichatterType == 0) { // it's not original smoothing because it works every time
		position.x += deltaX * weight;
		position.y += deltaY * weight;
	}
	// Devocub smoothing
	else {
		if (antichatterType == 1) // Increase weight of filter in {Strength} times when in {Range}
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
			if (antichatterType == 2) { // Increase weight of filter in {formula} times when in {Range}
				if (distance > antichatterRange) { // use regular smoothing when exceed distance (weightModifier = 1)
					position.x += deltaX * weight;
					position.y += deltaY * weight;
				}
				else { // 2nd type of smoothing
					weightModifier = pow((distance + antichatterOffsetX), antichatterStrength*-1)*antichatterMultiplier + antichatterOffsetY;
					if (weightModifier < 1) weightModifier = 1;
					position.x += deltaX * (weight / weightModifier);
					position.y += deltaY * (weight / weightModifier);
				}
			}
		else
			if (antichatterType == 3) { // Same as 2 except
				if (distance > antichatterRange) { // disable filtering when exceed distance
					position.x = target.x;
					position.y = target.y;
				}
				else { // same as 2
					weightModifier = pow((distance + antichatterOffsetX), antichatterStrength*-1)*antichatterMultiplier + antichatterOffsetY;
					if (weightModifier < 1) weightModifier = 1;
					position.x += deltaX * (weight / weightModifier);
					position.y += deltaY * (weight / weightModifier);
				}
			}
		else
			if (antichatterType == 4) { // Z ~= height, stength of signal from pen
				if (distance > antichatterRange) { 
					position.x += deltaX * weight;
					position.y += deltaY * weight;
				}
				else { // same as 2 * heightModifier
					// 480 z is 14-41
					// 470 z is 0-30
					if (z <= 5) heightModifier = 20;
					else if (z <= 10) heightModifier = 2;
					else if (z <= 15) heightModifier = 0.5;
					else if (z <= 20) heightModifier = 0.01;
					weightModifier = pow((distance + antichatterOffsetX), antichatterStrength*-1)*antichatterMultiplier*heightModifier + antichatterOffsetY;
					if (weightModifier < 1) weightModifier = 1;
					position.x += deltaX * (weight / weightModifier);
					position.y += deltaY * (weight / weightModifier);
				}
			}
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


