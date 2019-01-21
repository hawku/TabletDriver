#include "precompiled.h"
#include "TabletFilterAntiSmoothing.h"


#define LOG_MODULE "AntiSmoothing"
#include "Logger.h"


//
// Constructor
//
TabletFilterAntiSmoothing::TabletFilterAntiSmoothing() {

	// Default settings
	ClearSettings();
	AddSettings(0, 1.0, 1);

	onlyWhenHover = false;
	targetReportRate = 0;
	dragMultiplier = 1.0;

	reportRate = 100;
	reportRateAverage = 100;
	velocity = 0;
	acceleration = 0;

	oldVelocity = 0;
	oldAcceleration = 0;

	ignoreInvalidReports = 0;

	timeBegin = std::chrono::high_resolution_clock::now();
	timeLastReport = timeBegin;
	timeNow = timeBegin;

	outputPosition = &outputState.position;
}

//
// Destructor
//
TabletFilterAntiSmoothing::~TabletFilterAntiSmoothing() {
}


//
// Set target
//
void TabletFilterAntiSmoothing::SetTarget(TabletState *tabletState) {
	latestTarget.Set(tabletState->position);
	timeNow = tabletState->time;
	memcpy(&this->tabletState, tabletState, sizeof(TabletState));
	memcpy(&outputState, tabletState, sizeof(TabletState));
}


//
// Update filter
//
void TabletFilterAntiSmoothing::Update() {

	double timeDelta;
	Vector2D predictedPosition;
	double shape = 0;
	double compensation = 0;

	//
	// Report rate calculation (moving average)
	//
	timeDelta = (timeNow - timeLastReport).count() / 1000000.0;
	if(timeDelta >= 1 && timeDelta <= 10) {

		// Limits
		if(reportRate < 100) reportRate = 100;
		else if(reportRate > 1000) reportRate = 1000;
		if(reportRateAverage < 100) reportRateAverage = 100;
		else if(reportRateAverage > 1000) reportRateAverage = 1000;

		reportRateAverage += ((1000.0 / timeDelta) - reportRateAverage) * (timeDelta / 1000.0) * 10;
		if(reportRateAverage > reportRate) {
			reportRate = reportRateAverage;
		}
	}
	timeLastReport = timeNow;

	// Velocity and acceleration calculation
	velocity = latestTarget.Distance(oldTarget) * reportRate;
	acceleration = (velocity - oldVelocity) * reportRate;

	// Low speed -> do not filter
	if(velocity < 1) {
		return;
	}

	//
	// Invalid position data detection.
	// Some tablets do send invalid/broken data when buttons are pressed.
	//
	if(timeDelta > 10) {

		if(logger.IsDebugOutputEnabled()) {
			LOG_DEBUG("INVALID! V=%0.2f mm/s, A=%0.0f mm/s^2, D=%0.2f ms\n",
				velocity,
				acceleration,
				timeDelta
			);
		}
		ignoreInvalidReports = 2;
	}

	//
	// Drag multiplier
	//
	double compensationMultiplier = 1.0;
	if(tabletState.inputPressure > 0) {
		compensationMultiplier = dragMultiplier;
	}


	// Velocity prediction
	double predictedVelocity = velocity + acceleration / reportRate;

	//
	// Skip invalid reports by setting the position as latest target
	//
	if(ignoreInvalidReports > 0) {
		if(logger.IsDebugOutputEnabled()) {
			LOG_DEBUG("IGNORE! V=%0.2f mm/s, A=%0.0f mm/s^2, D=%0.2f ms\n",
				velocity,
				acceleration,
				timeDelta
			);
		}
		outputPosition->Set(latestTarget);
		ignoreInvalidReports--;
	}

	//
	// Position prediction
	//
	else if(
		velocity > 0.1 && predictedVelocity > 0.1
		&&
		velocity < 2000 && predictedVelocity < 2000
	) {
		//
		// Extrapolate a position by using the difference between predicted velocity and current velocity.
		// LerpAdd (linear interpolation) method will move the position beyond the target position when the second parameter is larger than 1
		//
		predictedPosition.Set(oldTarget);

		// Get settings
		GetSettingsByVelocity(velocity, &shape, &compensation);

		double shapedVelocityFactor = pow(predictedVelocity / velocity, shape) * compensationMultiplier;

		// Workaround for VEIKK S640 and other tablets with variable report rate and smoothing
		if(targetReportRate > 0) {
			predictedPosition.LerpAdd(latestTarget, 1 + shapedVelocityFactor * compensation * (targetReportRate / 1000.0));
		}

		// Other tablets
		else {
			predictedPosition.LerpAdd(latestTarget, 1 + shapedVelocityFactor * compensation * (reportRate / 1000.0));
		}

		/*
		if(latestTarget.Distance(predictedPosition) > 5) {
			LOG_DEBUG("LONG DELTA: %0.3f,%0.3f -> %0.3f,%0.3f V:%0.3f OV:%0.3f PV:%0.3f PV2:%0.3f R:%0.3f A:%0.3f, T:%lld\n",
				latestTarget.x,
				latestTarget.y,
				predictedPosition.x,
				predictedPosition.y,
				velocity,
				oldVelocity,
				predictedVelocity,
				velocity + (acceleration) / reportRate,
				reportRate,
				acceleration,
				tabletState.time
			);
		}
		*/

		// Set output position
		outputPosition->Set(predictedPosition);
	}

	// Invalid velocity -> set the position to the latest target
	else {
		outputPosition->Set(latestTarget);
	}


	//
	// Enabled only when hovering
	//
	if(onlyWhenHover) {

		// Ignore anti-smoothing pen pressure is detected
		if(tabletState.inputPressure > 0) {
			outputPosition->Set(latestTarget);
		}

		// Switching from drag to hover
		if(tabletState.inputPressure == 0 && oldTabletState.inputPressure > 0) {
			ignoreInvalidReports = 1;
			outputPosition->Set(latestTarget);
		}

		// Switching from hover to drag
		if(tabletState.inputPressure > 0 && oldTabletState.inputPressure == 0) {
			ignoreInvalidReports = 1;
			outputPosition->Set(latestTarget);
		}

	}

	// Debug message
	if(logger.IsDebugOutputEnabled()) {
		double delta = outputPosition->Distance(latestTarget);
		double pixelDensity = (mapper->primaryScreenArea->width / mapper->primaryTabletArea->width);
		LOG_DEBUG(
			//"T=%0.2f T=[%0.2f,%0.2f] P=[%0.2f,%0.2f] D1=%0.3f D2=%0.3f R=%0.2f RA=%0.2f V=%0.2f PV=%0.2f A=%0.0f L=%0.2f\n",
			"T=%0.2f D=%0.2f R=%0.0f V=%0.0f PV=%0.0f L=%0.2f ms LP=%0.0f pixels\n",
			(timeNow - timeBegin).count() / 1000000.0,
			//latestTarget.x, latestTarget.y,
			//outputPosition->x, outputPosition->y,
			delta,
			reportRate,
			//reportRateAverage,
			velocity,
			predictedVelocity,
			//acceleration,
			(velocity > 0) ? (delta / velocity * 1000) : 0,
			pixelDensity * delta
		);
	}

	// Update old values
	oldVelocity = velocity;
	oldAcceleration = acceleration;
	oldTarget.Set(latestTarget);
	memcpy(&this->oldTabletState, &this->tabletState, sizeof(TabletState));


}

//
// Add settings
//
void TabletFilterAntiSmoothing::AddSettings(double velocity, double shape, double compensation)
{
	if(settingCount >= (sizeof(settings) / sizeof(Settings))) return;
	int index = settingCount;
	settings[index].velocity = velocity;
	settings[index].shape = shape;
	settings[index].compensation = compensation;
	settingCount++;
}

//
// Clear settings
//
void TabletFilterAntiSmoothing::ClearSettings()
{
	settingCount = 0;
}

//
// Get settings by velocity
//
bool TabletFilterAntiSmoothing::GetSettingsByVelocity(double velocity, double *shape, double *compensation)
{
	Settings *lastSettings = NULL;
	// Loop through settings

	//double shape = 0.5;
	//double compensation = 0;

	for(int i = 0; i < settingCount; i++) {

		Settings *currentSettings = &settings[i];


		// First settings in the list
		if(lastSettings == NULL) {
			*shape = currentSettings->shape;
			*compensation = currentSettings->compensation;
		}

		// Velocity larger than current setting
		if(
			lastSettings != NULL
			&&
			velocity >= currentSettings->velocity
		) {
			*shape = currentSettings->shape;
			*compensation = currentSettings->compensation;
		}

		// Velocity between last and current settings
		if(
			lastSettings != NULL
			&&
			velocity >= lastSettings->velocity
			&&
			velocity <= currentSettings->velocity
		) {
			double position =
				(velocity - lastSettings->velocity)
				/
				(currentSettings->velocity - lastSettings->velocity);

			*shape =
				lastSettings->shape * (1 - position)
				+
				currentSettings->shape * position;

			*compensation =
				lastSettings->compensation * (1 - position)
				+
				currentSettings->compensation * position;

		}
		lastSettings = &settings[i];

	}



	return true;
}



