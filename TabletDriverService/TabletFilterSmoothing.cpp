#include "stdafx.h"
#include "TabletFilterSmoothing.h"

#define LOG_MODULE "Smoothing"
#include "Logger.h"

//
// Constructor
//
TabletFilterSmoothing::TabletFilterSmoothing() {
	latency = 2.0;
	weight = 1.000;
	threshold = 0.9;
	onlyWhenButtonsDown = false;
	lastTargetTime = chrono::high_resolution_clock::now();
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
void TabletFilterSmoothing::SetTarget(TabletState *tabletState) {

	// Set target state
	memcpy(&target, tabletState, sizeof(TabletState));

	// Set output state
	outputState.time = target.time;
	outputState.isValid = target.isValid;
	outputState.buttons = target.buttons;

	// Reset output position if last tablet state update is older than 100 milliseconds.
	double timeDelta = (tabletState->time - lastTargetTime).count() / 1000000.0;
	if(timeDelta > 100.0) {
		outputState.position.Set(tabletState->position);
		outputState.pressure = tabletState->pressure;
	}

	lastTargetTime = tabletState->time;
}

//
// Filter timer interval changed
//
void TabletFilterSmoothing::OnTimerIntervalChange(double oldInterval, double newInterval) {
	timerInterval = newInterval;
	SetLatency(latency);
}

// Update
void TabletFilterSmoothing::Update() {

	double timeDelta = (chrono::high_resolution_clock::now() - outputState.time).count() / 1000000.0;

	//
	// Do not run the filter if the last state is older than 100 milliseconds.
	//
	if(timeDelta > 100) {
		return;
	}

	double deltaX, deltaY, deltaPressure, distance;

	deltaX = target.position.x - outputState.position.x;
	deltaY = target.position.y - outputState.position.y;
	deltaPressure = target.pressure - outputState.pressure;
	distance = sqrt(deltaX * deltaX + deltaY * deltaY);
	
	// Move output position only when buttons are down
	if(onlyWhenButtonsDown) {

		// Move output values
		if((target.buttons & 0x1F) > 0) {
			outputState.position.x += deltaX * weight;
			outputState.position.y += deltaY * weight;
			outputState.pressure += deltaPressure * weight;
		}

		// Set output values same as target
		else {
			outputState.position.Set(target.position);
			outputState.pressure = target.pressure;
		}
	}

	// Move output values
	else {
		outputState.position.x += deltaX * weight;
		outputState.position.y += deltaY * weight;
		outputState.pressure += deltaPressure * weight;
	}

	// Debug message
	if(logger.debugEnabled) {
		LOG_DEBUG("TX=%0.2f TY=%0.2f TP=%0.2f OX=%0.2f OY=%0.2f OP=%0.2f DX=%0.2f DY=%0.2f DP=%0.2f D=%0.2f W=%0.3f\n",
			target.position.x,
			target.position.y,
			target.pressure,
			outputState.position.x,
			outputState.position.y,
			outputState.pressure,
			deltaX,
			deltaY,
			deltaPressure,
			distance,
			weight
		);
	}

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


