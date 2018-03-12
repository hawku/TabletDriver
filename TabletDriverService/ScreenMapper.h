#pragma once
#include "Tablet.h"

typedef struct {
	double x;
	double y;
	double width;
	double height;
} Area;

class ScreenMapper {
public:
	Tablet * tablet;
	Area areaTablet;
	Area areaScreen;
	Area areaVirtualScreen;
	double rotationMatrix[4];

	ScreenMapper(Tablet *t);
	void SetRotation(double angle);
	bool GetRotatedTabletPosition(double *x, double *y);
	bool GetScreenPosition(double *x, double *y);
};

