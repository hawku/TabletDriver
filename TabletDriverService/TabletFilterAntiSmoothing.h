#pragma once
#include "TabletFilter.h"
#include "PositionRingBuffer.h"

#include <chrono>


class TabletFilterAntiSmoothing : public TabletFilter {
private:
	double reportRate;
	double reportRateAverage;
	double velocity;
	double acceleration;
	double oldVelocity;
	double oldAcceleration;
	int ignoreInvalidReports;
	chrono::high_resolution_clock::time_point timeBegin;
	chrono::high_resolution_clock::time_point timeLastReport;
	chrono::high_resolution_clock::time_point timeNow;

public:
	Vector2D latestTarget;
	Vector2D oldTarget;
	Vector2D *outputPosition;

	TabletState tabletState;
	TabletState oldTabletState;

	class Settings {
	public:
		double velocity;
		double shape;
		double compensation;
	};
	Settings settings[32];
	int settingCount;

	bool onlyWhenHover;
	double targetReportRate;
	double dragMultiplier;

	void SetTarget(TabletState *tabletState);
	void Update();


	void AddSettings(double velocity, double shape, double compensation);
	void ClearSettings();
	bool GetSettingsByVelocity(double velocity, double *shape, double *compensation);

	TabletFilterAntiSmoothing();
	~TabletFilterAntiSmoothing();

};

