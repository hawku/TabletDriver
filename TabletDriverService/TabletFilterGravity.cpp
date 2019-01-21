#include "precompiled.h"
#include "TabletFilterGravity.h"

#define LOG_MODULE "Gravity"
#include "Logger.h"

TabletFilterGravity::TabletFilterGravity() {
	gravity = 10.0;
	friction = 10.0;
	pressureGravity = 0.0;
	pressureFriction = 0.0;

	velocityVector.Set(0, 0);
	outputPosition.Set(0, 0);
	ignoreButton = 0;

	isEnabled = false;
}


TabletFilterGravity::~TabletFilterGravity() {
}

void TabletFilterGravity::SetTarget(TabletState *tabletState) {
	targetPosition.Set(tabletState->position);
	memcpy(&outputState, tabletState, sizeof(TabletState));
}

void TabletFilterGravity::Update() {

	double timeDelta = timerInterval / 1000.0;

	//
	// Only use this filter when buttons are down
	//
	if((outputState.buttons & 0x1F) == 0) {
		if(ignoreButton-- > 0) {
		}
		else {
			outputPosition.Set(targetPosition);
			velocityVector.Set(0, 0);
		}
	}
	else {
		ignoreButton = 2;
	}

	// Target to output distance vector
	Vector2D distanceVector(
		targetPosition.x - outputPosition.x,
		targetPosition.y - outputPosition.y
	);
	double distance = distanceVector.Magnitude();


	//
	// Gravity calculation
	//
	double gravityForce = pow(distance, 0.5) * (gravity + outputState.pressure * pressureGravity) * timeDelta;
	Vector2D gravityVector(
		(distanceVector.x / distance) * gravityForce,
		(distanceVector.y / distance) * gravityForce
	);
	velocityVector.Add(gravityVector);


	//
	// Friction calculation
	//
	double velocityMagnitude = velocityVector.Magnitude();

	// Apply friction when the velocity is over 1 mm/s
	if(velocityMagnitude > 1.0 * timeDelta) {

		double frictionForce = velocityMagnitude * (friction + outputState.pressure * pressureFriction) * timeDelta;

		Vector2D frictionVector(
			-(velocityVector.x / velocityMagnitude) * frictionForce,
			-(velocityVector.y / velocityMagnitude) * frictionForce
		);
		velocityVector.Add(frictionVector);

	}
	else {
		velocityVector.Set(0, 0);
	}

	//
	// Move output position
	//
	outputPosition.Add(
		velocityVector.x * timeDelta,
		velocityVector.y * timeDelta
	);


	// Set output state
	outputState.position.Set(outputPosition);


	// Debug message
	if(logger.IsDebugOutputEnabled() && distance > 0.0) {
		LOG_DEBUG("X=%0.2f Y=%0.2f DX=%0.2f DY=%0.2f VX=%0.2f VY=%0.2f\n",
			outputPosition.x,
			outputPosition.y,
			distanceVector.x,
			distanceVector.y,
			velocityVector.x,
			velocityVector.y
		);
	}

}
