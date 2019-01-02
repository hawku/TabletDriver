#pragma once

#include "Vector2D.h"
#include <chrono>

class TabletState {
public:
	chrono::high_resolution_clock::time_point time;
	
	unsigned char inputButtons;
	Vector2D inputPosition;
	double inputPressure;
	double inputVelocity;

	unsigned char buttons;
	Vector2D position;
	double pressure;

	bool isValid;

	TabletState();
	~TabletState();
};

