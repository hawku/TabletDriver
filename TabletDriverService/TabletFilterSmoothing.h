#pragma once

#include "Vector2D.h"
#include "TabletFilter.h"

class TabletFilterSmoothing : public TabletFilter {
public:
	double latency;
	double weight;
	double threshold;
	TabletState target;
	std::chrono::high_resolution_clock::time_point lastTargetTime;
	bool onlyWhenButtonsDown;

	TabletFilterSmoothing();
	~TabletFilterSmoothing();

	void SetTarget(TabletState *tabletState);
	void OnTimerIntervalChange(double oldInterval, double newInterval) override;
	void Update();

	double GetLatency(double filterWeight, double interval, double threshold);
	double GetLatency(double filterWeight);
	double GetLatency();
	void SetLatency(double latency);

	double GetWeight(double latency, double interval, double threshold);
	double GetWeight(double latency);
};

