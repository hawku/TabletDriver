#pragma once
#include "TabletFilter.h"
class TabletFilterGravity :
	public TabletFilter {
public:
	Vector2D targetPosition;
	Vector2D outputPosition;
	Vector2D velocityVector;

	double gravity;
	double friction;
	double pressureGravity;
	double pressureFriction;

	int ignoreButton;

	TabletFilterGravity();
	~TabletFilterGravity();

	void SetTarget(TabletState *tabletState);
	void Update();
};

