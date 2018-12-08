#pragma once
#include "Vector2D.h"
#include "TabletState.h"
#include <chrono>

class TabletFilter {
public:

	TabletState outputState;
	double timerInterval;
	bool isEnabled;
	bool isValid;

	TabletFilter();

	virtual void SetTarget(TabletState *tabletState) = 0;
	virtual void SetOutput(TabletState *tabletState);
	virtual bool GetOutput(TabletState *tabletState);
	virtual void OnTimerIntervalChange(double oldInterval, double newInterval);
	virtual void Update() = 0;

};

