#pragma once
#include "Tablet.h"
#include "Vector2D.h"


class ScreenMapper {
public:
	class Area {
	public:
		double x;
		double y;
		double width;
		double height;

		double minX;
		double maxX;
		double minY;
		double maxY;

		void UpdateValues();
	};

	class ScreenMap {
	public:
		Area tablet;
		Area screen;
		Vector2D rotatedPosition;
		double distance;
		double rotationMatrix[4];
		void SetRotation(double angle);
		void UpdateValues();
	};

	Tablet *tablet;
	Area virtualScreen;
	ScreenMap screenMaps[10];
	int screenMapCount;
	ScreenMap *primaryMap;
	Area *primaryTabletArea;
	Area *primaryScreenArea;
	double *primaryRotationMatrix;

	ScreenMapper(Tablet *t);
	bool GetRotatedTabletPosition(double *x, double *y);
	bool GetScreenPosition(double *x, double *y);
	void UpdateValues();
};

