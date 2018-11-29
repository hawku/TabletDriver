#pragma once

#include "Vector2D.h"

class PositionRingBuffer {
public:
	Vector2D buffer[100];
	int maxLength;
	int length;
	int count;
	int index;
	bool isValid;

	void SetLength(int length);
	void Add(Vector2D vector);
	void LerpAdd(Vector2D target, double t);
	bool GetLatest(Vector2D *output, int delta);
	void Reset();

	Vector2D *operator[](std::size_t index);


	PositionRingBuffer();
	~PositionRingBuffer();
};

