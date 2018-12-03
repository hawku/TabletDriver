#pragma once
#include "Vector2D.h"
#include "TabletState.h"
#include <chrono>

class TabletFilter {
public:
	virtual void SetTarget(TabletState *tabletState) = 0;
	virtual void SetPosition(Vector2D vector) = 0;
	virtual bool GetPosition(Vector2D *vector) = 0;
	virtual void Update() = 0;

	HANDLE timer;
	WAITORTIMERCALLBACK callback;
	double timerInterval;

	bool isEnabled;
	bool isValid;

	TabletFilter();

	bool StartTimer();
	bool StopTimer();

};

