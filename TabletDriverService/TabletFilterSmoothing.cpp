#include "precompiled.h"
#include "TabletFilterSmoothing.h"

#define LOG_MODULE "Smoothing"
#include "Logger.h"

//
// Constructor
//
TabletFilterSmoothing::TabletFilterSmoothing() {
	latency = 2.0;
	weight = 1.000;
	threshold = 0.63;
	onlyWhenButtonsDown = false;
	lastTargetTime = std::chrono::high_resolution_clock::now();

	target.position.Set(0, 0);
	outputState.position.Set(0,0);
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

	Vector2D tmpPosition(
		outputState.position.x,
		outputState.position.y
	);
	double oldPressure = outputState.pressure;

	// Copy current state to output state
	memcpy(&outputState, tabletState, sizeof(TabletState));

	// Reset output position if last tablet state update is older than 100 milliseconds.
	double timeDelta = (tabletState->time - lastTargetTime).count() / 1000000.0;
	if(timeDelta > 100.0) {
		outputState.position.Set(tabletState->position);
		outputState.pressure = tabletState->pressure;
	}

	// Set output state position and pressure
	else {
		outputState.position.Set(tmpPosition);
		outputState.pressure = oldPressure;
	}

	// 
	lastTargetTime = tabletState->time;
}

//
// Filter timer interval changed
//
void TabletFilterSmoothing::OnTimerIntervalChange(double oldInterval, double newInterval) {
	timerInterval = newInterval;

	// TODO: Find a better way to calculate this...
	//threshold = 0.90;
	threshold = 0.63;

	SetLatency(latency);
}

// Update
void TabletFilterSmoothing::Update() {

	if(!target.isValid) return;

	double timeDelta = (std::chrono::high_resolution_clock::now() - outputState.time).count() / 1000000.0;

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
		if(target.buttons > 0) {
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

	double dx = deltaX * weight;
	double dy = deltaY * weight;
	double velocity = sqrt(dx * dx + dy * dy) * (1000.0 / timerInterval);

	// Debug message
	if(logger.IsDebugOutputEnabled()) {
		LOG_DEBUG("TXY=%0.2f,%0.2f TP=%0.2f OXY=%0.2f,%0.2f OP=%0.2f DXY=%0.2f,%0.2f DP=%0.2f D=%0.2f W=%0.3f V=%0.2f L=%0.2f\n",
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
			weight,
			velocity,
			(distance / velocity) * 1000.0
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

// Set Latency
void TabletFilterSmoothing::SetLatency(double latency) {
	this->weight = GetWeight(latency);
	this->latency = latency;
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



