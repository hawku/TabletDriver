#include "stdafx.h"
#include "TabletFilterAntiSmoothing.h"


#define LOG_MODULE "AntiSmoothing"
#include "Logger.h"


//
// Constructor
//
TabletFilterAntiSmoothing::TabletFilterAntiSmoothing() {
	power = 1.0;
	multiplier = 1;

	reportRate = 1;
	velocity = 0;
	acceleration = 0;
	jerk = 0;

	lastVelocity = 0;
	lastAcceleration = 0;
	lastJerk = 0;

	timeBegin = chrono::high_resolution_clock::now();
	timeLastReport = timeBegin;
	timeNow = timeBegin;
}

//
// Destructor
//
TabletFilterAntiSmoothing::~TabletFilterAntiSmoothing() {
}

//
// TabletFilter methods
//

// Set target position
void TabletFilterAntiSmoothing::SetTarget(Vector2D targetVector) {
	latestTarget.Set(targetVector);
	buffer.Add(targetVector);
}

// Set position
void TabletFilterAntiSmoothing::SetPosition(Vector2D vector) {
	position.x = vector.x;
	position.y = vector.y;
}

// Get position
bool TabletFilterAntiSmoothing::GetPosition(Vector2D *outputVector) {
	outputVector->x = position.x;
	outputVector->y = position.y;
	return true;
}

// Update
void TabletFilterAntiSmoothing::Update() {

	Vector2D predictedPosition;

	// Report rate calculation (moving average)
	timeNow = chrono::high_resolution_clock::now();
	double timeDelta = (timeNow - timeLastReport).count() / 1000000.0;
	if(timeDelta >= 1 && timeDelta <= 10) {
		reportRate += ((1000.0 / timeDelta) - reportRate) * (timeDelta / 1000.0) / 0.1;
	} else {
		timeLastReport = timeNow;
		return;
	}
	timeLastReport = timeNow;

	// Velocity, acceleration and jerk calculation
	velocity = latestTarget.Distance(lastTarget) * reportRate;
	acceleration = (velocity - lastVelocity) * reportRate;
	jerk = (acceleration - lastAcceleration) * reportRate;

	// Last values
	lastVelocity = velocity;
	lastAcceleration = acceleration;
	lastJerk = jerk;

	// Velocity prediction
	double predictedVelocity = velocity + (acceleration + jerk / reportRate) / reportRate;

	// Velocity validation
	if(
		velocity > 0.1 && predictedVelocity > 0.1
		&&
		velocity < 2000 && predictedVelocity < 2000
	) {
		//
		// Extrapolate a position by using the difference between predicted velocity and current velocity.
		// LerpAdd (linear interpolation) method will move the position beyond the target position when the second parameter is larger than 1
		//
		// Power: Exponential prediction modifier
		// Multiplier: Linear prediction modifier
		// 
		predictedPosition.Set(lastTarget);
		predictedPosition.LerpAdd(latestTarget, pow(predictedVelocity / velocity * multiplier, power));
		position.Set(predictedPosition);

	// Invalid velocity -> set the position to the latest target
	} else {
		position.Set(latestTarget);
	}


	// Debug message
	if(tablet->debugEnabled) {
		double delta = position.Distance(latestTarget);
		LOG_DEBUG("T=%0.0f B=%d T=[%0.2f,%0.2f] P=[%0.2f,%0.2f] D=%0.2f R=%0.2f V=%0.2f PV=%0.2f A=%0.0f J=%0.0f\n",
			(timeNow - timeBegin).count() / 1000000.0,
			buffer.count,
			latestTarget.x, latestTarget.y,
			position.x, position.y,
			delta,
			reportRate,
			velocity,
			predictedVelocity,
			acceleration,
			jerk
		);
	}

	// Set last target
	lastTarget.Set(latestTarget);



}



