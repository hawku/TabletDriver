#pragma once
class TabletFilter {
public:
	virtual void SetTarget(Vector2D vector, double h) = 0;
	virtual void Reset(Vector2D vector) = 0;
	virtual void SetPosition(Vector2D vector, double h) = 0;
	virtual bool GetPosition(Vector2D *vector) = 0;
	virtual void Update() = 0;

	HANDLE timer;
	UINT uTimerID;
	LPTIMECALLBACK callback;
	double timerInterval;

	bool isEnabled;
	bool isValid;

	TabletFilter();

	bool StartTimer();
	bool StopTimer();

};

