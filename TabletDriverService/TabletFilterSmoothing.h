#pragma once

#include "Vector2D.h"
#include "TabletFilter.h"

class TabletFilterSmoothing : public TabletFilter {
public:
	double latency;
	double weight;
	double threshold;

	bool AntichatterEnabled;
	double antichatterStrength;
	double antichatterMultiplier;
	double antichatterOffsetX;
	double antichatterOffsetY;

	bool PredictionEnabled;
	double PredictionSharpness;
	double PredictionStrength;
	double PredictionOffsetX;
	double PredictionOffsetY;

	Vector2D target;
	Vector2D prev_target;
	Vector2D calculated_target;
	Vector2D position;
	double z;

	TabletFilterSmoothing();
	~TabletFilterSmoothing();

	void SetTarget(Vector2D vector, double h);
	void SetPosition(Vector2D vector, double h);
	bool GetPosition(Vector2D *outputVector);
	void Update();

	double SetPosition(double x, double y, double h);
	double GetLatency(double filterWeight, double interval, double threshold);
	double GetLatency(double filterWeight);
	double GetLatency();
	double GetWeight(double latency, double interval, double threshold);
	double GetWeight(double latency);
	void SetLatency(double latency);
};

