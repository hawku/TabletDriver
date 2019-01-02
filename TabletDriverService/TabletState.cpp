#include "stdafx.h"
#include "TabletState.h"


TabletState::TabletState() {
	time = chrono::high_resolution_clock::now();
	
	inputButtons = 0;
	inputPosition.Set(0,0);
	inputPressure = 0;
	inputVelocity = 0;

	buttons = 0;
	position.Set(0,0);
	pressure = 0;

	isValid = false;
}


TabletState::~TabletState() {
}
