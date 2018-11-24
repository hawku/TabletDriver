#include "stdafx.h"
#include "TabletState.h"


TabletState::TabletState() {
	buttons = 0;
	position.x = 0;
	position.y = 0;
	pressure = 0;
	isValid = false;
}


TabletState::~TabletState() {
}
