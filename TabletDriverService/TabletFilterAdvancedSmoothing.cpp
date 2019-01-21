#include "precompiled.h"
#include "TabletFilterAdvancedSmoothing.h"

#define LOG_MODULE "AdvanceSmoothing"
#include "Logger.h"

//
// Constructor
//
TabletFilterAdvancedSmoothing::TabletFilterAdvancedSmoothing() {

	isEnabled = false;
	timerInterval = 1.0;
	threshold = 0.63;
	velocity = 0;

	// Default settings
	ClearSettings();
	for(int i = 0; i < 4; i++) {
		AddSettings(
			i * 100,
			i >= 2 ? true : false,
			1.000
		);
	}

	lastTargetTime = std::chrono::high_resolution_clock::now();
}


//
// Destructor
//
TabletFilterAdvancedSmoothing::~TabletFilterAdvancedSmoothing() {
}


//
// TabletFilter methods
//

// Set target position
void TabletFilterAdvancedSmoothing::SetTarget(TabletState *tabletState) {

	// Set target state
	memcpy(&target, tabletState, sizeof(TabletState));

	Vector2D oldPosition(
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
		outputState.position.Set(oldPosition);
		outputState.pressure = oldPressure;
	}

	// Last target
	if(tabletState->time != lastTarget.time) {
		timeDelta = (tabletState->time - lastTarget.time).count() / 1000000.0;
		velocity = lastTarget.position.Distance(target.position) * (1000.0 / timeDelta);
		memcpy(&lastTarget, tabletState, sizeof(TabletState));
	}

	// 
	lastTargetTime = tabletState->time;

}

//
// Filter timer interval changed
//
void TabletFilterAdvancedSmoothing::OnTimerIntervalChange(double oldInterval, double newInterval) {
	timerInterval = newInterval;

	// TODO: Find a better way to calculate this...
	threshold = 0.63;

	// Update setting values
	for(int i = 0; i < settingCount; i++) {
		settings[i].interval = newInterval;
		settings[i].UpdateValues();
	}
}

// Update
void TabletFilterAdvancedSmoothing::Update() {

	double timeDelta = (std::chrono::high_resolution_clock::now() - outputState.time).count() / 1000000.0;

	//
	// Do not run the filter if the last state is older than 100 milliseconds.
	//
	if(timeDelta > 100) {
		return;
	}

	double weight;
	double deltaX, deltaY, deltaPressure, distance;
	bool dragging;

	deltaX = target.position.x - outputState.position.x;
	deltaY = target.position.y - outputState.position.y;
	deltaPressure = target.pressure - outputState.pressure;
	distance = sqrt(deltaX * deltaX + deltaY * deltaY);

	//velocity = lastPosition.Distance(target.position) * (1000.0 / timerInterval);

	dragging = (target.inputPressure > 0) ? true : false;
	weight = GetWeightByVelocity(velocity, dragging);

	outputState.position.x += deltaX * weight;
	outputState.position.y += deltaY * weight;
	outputState.pressure += deltaPressure * weight;

	// Debug message
	if(logger.IsDebugOutputEnabled()) {
		LOG_DEBUG("TX=%0.2f TY=%0.2f TP=%0.2f OX=%0.2f OY=%0.2f OP=%0.2f DX=%0.2f DY=%0.2f DP=%0.2f D=%0.2f W=%0.3f V=%0.2f mm/s\n",
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
			velocity
		);
	}
}

//
// Add smoothing setting
//
void TabletFilterAdvancedSmoothing::AddSettings(double velocity, bool dragging, double latency)
{
	if(settingCount >= (sizeof(settings) / sizeof(Settings))) return;
	int index = settingCount;

	settings[index].velocity = velocity;
	settings[index].dragging = dragging;
	settings[index].interval = timerInterval;
	settings[index].threshold = threshold;
	settings[index].latency = latency;
	settings[index].UpdateValues();


	settingCount++;

}

void TabletFilterAdvancedSmoothing::ClearSettings()
{
	settingCount = 0;
}

//
// Get interpolated weight by velocity
//
double TabletFilterAdvancedSmoothing::GetWeightByVelocity(double velocity, bool dragging)
{
	Settings *lastSettings[2];
	lastSettings[false] = NULL;
	lastSettings[true] = NULL;


	double weight = 1;

	// Loop through settings
	for(int i = 0; i < settingCount; i++) {

		Settings *currentSettings = &settings[i];

		// Hover or dragging?
		if(currentSettings->dragging == dragging) {

			// First setting
			if(lastSettings[dragging] == NULL) {
				weight = currentSettings->weight;
			}

			// Velocity larger than current setting
			if(
				lastSettings[dragging] != NULL
				&&
				velocity >= currentSettings->velocity
			) {
				weight = currentSettings->weight;
			}

			// Velocity between last and current settings
			if(
				lastSettings[dragging] != NULL
				&&
				velocity >= lastSettings[dragging]->velocity
				&&
				velocity <= currentSettings->velocity
			) {
				double position =
					(velocity - lastSettings[dragging]->velocity)
					/
					(currentSettings->velocity - lastSettings[dragging]->velocity);

				weight =
					lastSettings[dragging]->weight * (1 - position)
					+
					currentSettings->weight * position;
			}
			lastSettings[dragging] = &settings[i];
		}

	}
	return weight;

}




//
// Calculate filter latency
//
double TabletFilterAdvancedSmoothing::Settings::GetLatency(double filterWeight, double interval, double threshold) {
	double target = 1 - threshold;
	double stepCount = -log(1 / target) / log(1 - filterWeight);
	return stepCount * interval;
}
double TabletFilterAdvancedSmoothing::Settings::GetLatency(double interval, double threshold) {
	return GetLatency(weight, interval, threshold);
}

double TabletFilterAdvancedSmoothing::Settings::GetLatency()
{
	return GetLatency(interval, threshold);
}

void TabletFilterAdvancedSmoothing::Settings::SetLatency(double latency)
{
	this->latency = latency;
	this->weight = GetWeight(interval, threshold);
}


//
// Calculate filter weight
//
double TabletFilterAdvancedSmoothing::Settings::GetWeight(double latency, double interval, double threshold) {
	double stepCount = latency / interval;
	double target = 1 - threshold;
	return 1 - 1 / pow(1 / target, 1 / stepCount);
}
double TabletFilterAdvancedSmoothing::Settings::GetWeight(double interval, double threshold) {
	return GetWeight(latency, interval, threshold);
}

double TabletFilterAdvancedSmoothing::Settings::GetWeight()
{
	return GetWeight(interval, threshold);
}


//
// Update values
// 
void TabletFilterAdvancedSmoothing::Settings::UpdateValues()
{
	weight = GetWeight();
}

