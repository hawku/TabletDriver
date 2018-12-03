#pragma once
class Vector2D {
public:
	double x;
	double y;
	Vector2D();
	~Vector2D();

	void Set(double x, double y);
	void Set(Vector2D vector);
	void Add(double x, double y);
	void Add(Vector2D vector);
	void Multiply(double value);
	double Distance(Vector2D target);
	double Angle(Vector2D target);
	void LerpAdd(Vector2D target, double t);
};

