#pragma once

#include "Vector2D.h"
#include <chrono>

class TabletState {
public:
	std::chrono::high_resolution_clock::time_point time;
	
	unsigned char inputButtons;
	Vector2D inputPosition;
	double inputPressure;
	double inputHeight;
	double inputVelocity;

	unsigned char buttons;
	unsigned char lastButtons;
	Vector2D position;
	double pressure;
	double height;

	bool isValid;

	TabletState();
	~TabletState();
};

