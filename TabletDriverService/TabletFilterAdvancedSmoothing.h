#pragma once

#include "Vector2D.h"
#include "TabletFilter.h"

class TabletFilterAdvancedSmoothing : public TabletFilter {
public:
	class Settings {
	public:
		double velocity;
		bool dragging;

		double latency;
		double weight;
		double interval;
		double threshold;

		double GetLatency(double filterWeight, double interval, double threshold);
		double GetLatency(double interval, double threshold);
		double GetLatency();
		void SetLatency(double latency);

		double GetWeight(double latency, double interval, double threshold);
		double GetWeight(double interval, double threshold);
		double GetWeight();

		void UpdateValues();
	};

	Settings settings[32];
	int settingCount;
	double threshold;
	double velocity;

	TabletState target;
	TabletState lastTarget;
	std::chrono::high_resolution_clock::time_point lastTargetTime;

	TabletFilterAdvancedSmoothing();
	~TabletFilterAdvancedSmoothing();

	void SetTarget(TabletState *tabletState);
	void OnTimerIntervalChange(double oldInterval, double newInterval) override;
	void Update();

	void AddSettings(double velocity, bool dragging, double latency);
	void ClearSettings();
	double GetWeightByVelocity(double velocity, bool dragging);


};

