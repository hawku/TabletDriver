#include "precompiled.h"
#include "TabletState.h"


TabletState::TabletState() {
	time = std::chrono::high_resolution_clock::now();
	
	inputButtons = 0;
	inputPosition.Set(0,0);
	inputPressure = 0;
	inputVelocity = 0;

	buttons = 0;
	position.Set(0,0);
	pressure = 0;
	height = 0;

	isValid = false;
}


TabletState::~TabletState() {
}
