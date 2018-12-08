#include "stdafx.h"
#include "ScreenMapper.h"

#define LOG_MODULE "ScreenMapper"
#include "Logger.h"

#include <stdio.h>

#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>

ScreenMapper::ScreenMapper(Tablet *t) {
	this->tablet = t;

	// Default Virtual Screen Area
	areaVirtualScreen.width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	areaVirtualScreen.height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	areaVirtualScreen.x = 0;
	areaVirtualScreen.y = 0;

	// Default Tablet Area
	areaTablet.width = 80;
	areaTablet.height = 45;
	areaTablet.x = 10;
	areaTablet.y = 10;


	// Default Screen Area
	areaScreen.width = 1920;
	areaScreen.height = 1080;
	areaScreen.x = 0;
	areaScreen.y = 0;

	// Default Rotation Matrix
	rotationMatrix[0] = 1;
	rotationMatrix[1] = 0;
	rotationMatrix[2] = 0;
	rotationMatrix[3] = 1;


}

//
// Create rotation matrix
//
void ScreenMapper::SetRotation(double angle) {
	angle *= M_PI / 180;
	rotationMatrix[0] = cos(angle);
	rotationMatrix[1] = -sin(angle);
	rotationMatrix[2] = sin(angle);
	rotationMatrix[3] = cos(angle);
}

//
// Get rotated tablet position
//
bool ScreenMapper::GetRotatedTabletPosition(double *x, double *y) {
	double mapX, mapY;
	double tmpX, tmpY;

	mapX = (*x);
	mapY = (*y);

	// Offset tablet so that the center is at zero
	mapX -= tablet->settings.width / 2.0;
	mapY -= tablet->settings.height / 2.0;

	// Rotate
	tmpX = mapX;
	tmpY = mapY;
	mapX = tmpX * rotationMatrix[0] + tmpY * rotationMatrix[1];
	mapY = tmpX * rotationMatrix[2] + tmpY * rotationMatrix[3];

	// Offset back to center from zero
	mapX += tablet->settings.width / 2.0;
	mapY += tablet->settings.height / 2.0;

	// Set pointer values
	*x = mapX;
	*y = mapY;

	return true;
}

//
// Get screen position. Return values between 0 and 1
//
bool ScreenMapper::GetScreenPosition(double *x, double *y) {
	double mapX, mapY;
	double tmpX, tmpY;

	mapX = (*x);
	mapY = (*y);

	// Offset tablet area position
	mapX -= areaTablet.x;
	mapY -= areaTablet.y;

	// Rotate
	tmpX = mapX;
	tmpY = mapY;
	mapX = tmpX * rotationMatrix[0] + tmpY * rotationMatrix[1];
	mapY = tmpX * rotationMatrix[2] + tmpY * rotationMatrix[3];

	// Offset half of tablet area size
	mapX += areaTablet.width / 2.0;
	mapY += areaTablet.height / 2.0;

	// Normalize tablet area
	mapX /= areaTablet.width;
	mapY /= areaTablet.height;


	// Scale to screen area size
	mapX *= (areaScreen.width);
	mapY *= (areaScreen.height);

	// Offset screen area
	mapX += areaScreen.x;
	mapY += areaScreen.y;

	// Limit cursor to screen area
	if(mapX < areaScreen.x + 1) mapX = areaScreen.x + 1;
	if(mapY < areaScreen.y + 1) mapY = areaScreen.y + 1;
	if(mapX > areaScreen.x + areaScreen.width) mapX = areaScreen.x + areaScreen.width;
	if(mapY > areaScreen.y + areaScreen.height) mapY = areaScreen.y + areaScreen.height;


	// Normalize screen area
	mapX /= areaVirtualScreen.width;
	mapY /= areaVirtualScreen.height;

	// Limit values
	if(mapX > 1) mapX = 1;
	if(mapY > 1) mapY = 1;

	// Set pointer values
	*x = mapX;
	*y = mapY;

	return true;
}