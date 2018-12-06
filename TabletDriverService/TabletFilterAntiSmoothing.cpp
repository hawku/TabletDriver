#include "stdafx.h"
#include "TabletFilterAntiSmoothing.h"


#define LOG_MODULE "AntiSmoothing"
#include "Logger.h"


//
// Constructor
//
TabletFilterAntiSmoothing::TabletFilterAntiSmoothing() {
	shape = 1.0;
	compensation = 1;
	ignoreWhenDragging = false;

	reportRate = 100;
	reportRateAverage = 100;
	velocity = 0;
	acceleration = 0;
	jerk = 0;

	oldVelocity = 0;
	oldAcceleration = 0;
	oldJerk = 0;

	ignoreInvalidReports = 0;

	timeBegin = chrono::high_resolution_clock::now();
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

	// Report rate calculation (moving average)
	timeDelta = (timeNow - timeLastReport).count() / 1000000.0;
	if(timeDelta >= 1 && timeDelta <= 10) {
		reportRateAverage += ((1000.0 / timeDelta) - reportRateAverage) * (timeDelta / 1000.0) * 10;
		if(reportRateAverage > reportRate) {
			reportRate = reportRateAverage;
		} else {
			reportRate -= reportRate * (timeDelta / 1000.0) * 0.1;
		}
	} else {
		//timeLastReport = timeNow;
		//return;
	}
	timeLastReport = timeNow;

	// Velocity, acceleration and jerk calculation
	velocity = latestTarget.Distance(oldTarget) * reportRate;
	acceleration = (velocity - oldVelocity) * reportRate;
	jerk = (acceleration - oldAcceleration) * reportRate;

	//
	// Invalid position data detection.
	// Some tablets do send invalid/broken data when buttons are pressed.
	//
	// TODO: Improve this...
	//
	if(
		timeDelta > 10
		//||
		//(abs(acceleration) > 30000 && abs(jerk) > 10000000)
	) {

		if(logger.debugEnabled) {
			LOG_DEBUG("INVALID! V=%0.2f mm/s, A=%0.0f mm/s^2, J=%0.0f mm/s^3 D=%0.2f ms\n",
				velocity,
				acceleration,
				jerk,
				timeDelta
			);
		}
		ignoreInvalidReports = 2;
	}


	// Velocity prediction
	double predictedVelocity = velocity + (acceleration + jerk / reportRate) / reportRate;

	// Skip invalid reports by setting the position as latest target
	if(ignoreInvalidReports > 0) {
		if(logger.debugEnabled) {
			LOG_DEBUG("IGNORE! V=%0.2f mm/s, A=%0.0f mm/s^2, J=%0.0f mm/s^3 D=%0.2f ms\n",
				velocity,
				acceleration,
				jerk,
				timeDelta
			);
		}
		outputPosition->Set(latestTarget);
		ignoreInvalidReports--;


	// Velocity validation
	} else if(
		velocity > 0.1 && predictedVelocity > 0.1
		&&
		velocity < 2000 && predictedVelocity < 2000
	) {
		//
		// Extrapolate a position by using the difference between predicted velocity and current velocity.
		// LerpAdd (linear interpolation) method will move the position beyond the target position when the second parameter is larger than 1
		//
		predictedPosition.Set(oldTarget);
		predictedPosition.LerpAdd(latestTarget, pow(predictedVelocity / velocity, shape) * compensation);
		outputPosition->Set(predictedPosition);

	// Invalid velocity -> set the position to the latest target
	} else {
		outputPosition->Set(latestTarget);
	}


	//
	// Ignore filter when dragging
	//
	if(ignoreWhenDragging) {

		// Ignore anti-smoothing pen pressure is detected
		if(tabletState.pressure > 0) {
			outputPosition->Set(latestTarget);
		}

		// Switching from drag to hover
		if(tabletState.pressure == 0 && oldTabletState.pressure > 0) {
			ignoreInvalidReports = 1;
			outputPosition->Set(latestTarget);
		}

		// Switching from hover to drag
		if(tabletState.pressure > 0 && oldTabletState.pressure == 0) {
			ignoreInvalidReports = 1;
			outputPosition->Set(latestTarget);
		}

	}

	// Debug message
	if(logger.debugEnabled) {
		double delta = outputPosition->Distance(latestTarget);
		LOG_DEBUG("T=%0.2f T=[%0.2f,%0.2f] P=[%0.2f,%0.2f] D=%0.3f R=%0.2f RA=%0.2f V=%0.2f PV=%0.2f A=%0.0f J=%0.0f L=%0.2f\n",
			(timeNow - timeBegin).count() / 1000000.0,
			latestTarget.x, latestTarget.y,
			outputPosition->x, outputPosition->y,
			delta,
			reportRate,
			reportRateAverage,
			velocity,
			predictedVelocity,
			acceleration,
			jerk,
			(velocity > 0) ? (delta / velocity * 1000) : 0
		);
	}

	// Update old values
	oldVelocity = velocity;
	oldAcceleration = acceleration;
	oldJerk = jerk;
	oldTarget.Set(latestTarget);
	memcpy(&this->oldTabletState, &this->tabletState, sizeof(TabletState));


}



