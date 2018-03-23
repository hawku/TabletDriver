#pragma once
class TabletFilterNoiseReduction : public TabletFilter {
public:
	Vector2D buffer[100];
	int bufferMaxLength;
	int bufferLength;
	int bufferPositionCount;
	int bufferCurrentIndex;
	Vector2D position;
	Vector2D lastTarget;
	bool isValid;

	int iterations;
	double distanceThreshold;

	void SetTarget(Vector2D targetVector);
	void SetPosition(Vector2D vector);
	bool GetPosition(Vector2D *outputVector);
	void Update();

	void SetBufferLength(int length);
	void ResetBuffer();
	void AddBuffer(Vector2D vector);
	bool GetAverageVector(Vector2D *output);
	bool GetGeometricMedianVector(Vector2D *output, int iterations);

	TabletFilterNoiseReduction();
	~TabletFilterNoiseReduction();
};

