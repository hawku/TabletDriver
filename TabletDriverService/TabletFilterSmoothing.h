#pragma once

#include "Vector2D.h"
#include "TabletFilter.h"

class TabletFilterSmoothing : public TabletFilter {
public:
	double latency;
	double weight;
	double threshold;
	Vector2D target;
	Vector2D outputPosition;

	TabletFilterSmoothing();
	~TabletFilterSmoothing();

	void SetTarget(TabletState *tabletState);
	void OnTimerIntervalChange(double oldInterval, double newInterval) override;
	void Update();



	double GetLatency(double filterWeight, double interval, double threshold);
	double GetLatency(double filterWeight);
	double GetLatency();
	double GetWeight(double latency, double interval, double threshold);
	double GetWeight(double latency);
	void SetLatency(double latency);
};

