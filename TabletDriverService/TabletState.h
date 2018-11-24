#pragma once

#include "Vector2D.h"

class TabletState {
public:
	unsigned char buttons;
	Vector2D position;
	double pressure;
	bool isValid;

	TabletState();
	~TabletState();
};

