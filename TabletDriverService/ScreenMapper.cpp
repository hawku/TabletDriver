#include "precompiled.h"
#include "ScreenMapper.h"

#define LOG_MODULE "ScreenMapper"
#include "Logger.h"

#include <stdio.h>

#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>

ScreenMapper::ScreenMapper(Tablet *t) {
	this->tablet = t;

	screenMapCount = 1;
	primaryMap = &screenMaps[0];
	primaryTabletArea = &screenMaps[0].tablet;
	primaryScreenArea = &screenMaps[0].screen;
	primaryRotationMatrix = screenMaps[0].rotationMatrix;

	// Default Virtual Screen Area
	virtualScreen.width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	virtualScreen.height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	virtualScreen.x = virtualScreen.width / 2.0;
	virtualScreen.y = virtualScreen.height / 2.0;

	// Reset all screen maps
	for(ScreenMap &map : screenMaps) {

		// Tablet area
		map.tablet.width = 80;
		map.tablet.height = 80 * (virtualScreen.height / virtualScreen.width);
		map.tablet.x = map.tablet.width / 2.0;
		map.tablet.y = map.tablet.height / 2.0;

		// Screen area
		map.screen.width = virtualScreen.width;
		map.screen.height = virtualScreen.height;
		map.screen.x = map.screen.width / 2.0;
		map.screen.y = map.screen.height / 2.0;

		// Rotation
		map.SetRotation(0);
	}

	UpdateValues();

}

//
// Get rotated tablet position
//
bool ScreenMapper::GetRotatedTabletPosition(double *x, double *y) {
	Vector2D tmpPosition;
	Vector2D position(
		*x,
		*y
	);

	// Offset tablet so that the center is at zero
	position.x -= tablet->settings.width / 2.0;
	position.y -= tablet->settings.height / 2.0;

	// Rotate
	tmpPosition.Set(position);
	position.x = tmpPosition.x * primaryMap->rotationMatrix[0] + tmpPosition.y * primaryMap->rotationMatrix[1];
	position.y = tmpPosition.x * primaryMap->rotationMatrix[2] + tmpPosition.y * primaryMap->rotationMatrix[3];

	// Offset back to center from zero
	position.x += tablet->settings.width / 2.0;
	position.y += tablet->settings.height / 2.0;

	// Set pointer values
	*x = position.x;
	*y = position.y;

	return true;
}

//
// Get screen position. Return values between 0 and 1
//
bool ScreenMapper::GetScreenPosition(double *x, double *y) {
	Vector2D position;
	Vector2D tmpPosition;
	ScreenMap *screenMap;
	double minimumDistance = 99999;
	int matchingScreenMapIndex = 0;

	//
	// Loop through screen maps
	//
	for(int index = 0; index < screenMapCount; index++) {

		screenMap = &screenMaps[index];

		position.x = *x;
		position.y = *y;

		// Offset tablet area position
		position.x -= screenMap->tablet.x;
		position.y -= screenMap->tablet.y;

		// Rotate
		tmpPosition.Set(position);
		position.x = tmpPosition.x * screenMap->rotationMatrix[0] + tmpPosition.y * screenMap->rotationMatrix[1];
		position.y = tmpPosition.x * screenMap->rotationMatrix[2] + tmpPosition.y * screenMap->rotationMatrix[3];

		// Offset half of tablet area size
		position.x += screenMap->tablet.width / 2.0;
		position.y += screenMap->tablet.height / 2.0;

		// Update last rotated position
		screenMap->rotatedPosition.Set(position);


		//
		// Is the pen position inside of a tablet area?
		//
		if(
			position.x >= 0 && position.x <= screenMap->tablet.width
			&&
			position.y >= 0 && position.y <= screenMap->tablet.height
		) {
			matchingScreenMapIndex = index;
			screenMap->distance = 0;
			minimumDistance = 0;
			break;
		}

		//
		// Calculate distance from the tablet area
		//
		else {
			double distanceX = 0;
			double distanceY = 0;
			double tmp = 0;

			// X axis distance
			if(position.x < 0) {
				tmp = -position.x;
				if(tmp > distanceX)
					distanceX = tmp;
			}
			else if(position.x > screenMap->tablet.width) {
				tmp = position.x - screenMap->tablet.width;
				if(tmp > distanceX)
					distanceX = tmp;
			}

			// Y axis distance
			if(position.y < 0) {
				tmp = -position.y;
				if(tmp > distanceY)
					distanceY = tmp;
			}
			else if(position.y > screenMap->tablet.height) {
				tmp = position.y - screenMap->tablet.height;
				if(tmp > distanceY)
					distanceY = tmp;
			}

			// Distance
			screenMap->distance = sqrt(distanceX * distanceX + distanceY * distanceY);
		}

		// Update minimum distance
		if(screenMap->distance < minimumDistance) minimumDistance = screenMap->distance;

	}


	//
	// Pen position is not inside of a tablet area
	// 
	if(minimumDistance > 0) {

		// Set primary screen map as default
		screenMap = &screenMaps[0];

		//
		// Find the closest screen map
		//
		for(int index = 0; index < screenMapCount; index++) {
			if(screenMaps[index].distance <= minimumDistance) {
				screenMap = &screenMaps[index];
				position.Set(screenMaps[index].rotatedPosition);
				break;
			}
		}
	}

	// Set screen map to one that directly matches with the pen position
	else {
		screenMap = &screenMaps[matchingScreenMapIndex];
	}

	// Normalize tablet area
	position.x /= screenMap->tablet.width;
	position.y /= screenMap->tablet.height;

	// Scale to screen area size
	position.x *= (screenMap->screen.width);
	position.y *= (screenMap->screen.height);

	// Limit cursor to screen area
	if(position.x < 1) position.x = 1;
	else if(position.x > screenMap->screen.width) position.x = screenMap->screen.width;
	if(position.y < 1) position.y = 1;
	else if(position.y > screenMap->screen.height) position.y = screenMap->screen.height;

	// Offset screen area
	position.x += screenMap->screen.minX;
	position.y += screenMap->screen.minY;

	// Normalize virtual screen area
	position.x /= virtualScreen.width;
	position.y /= virtualScreen.height;

	// Limit values
	if(position.x > 1) position.x = 1;
	if(position.y > 1) position.y = 1;

	// Set output values
	*x = position.x;
	*y = position.y;

	return true;
}

//
// Update precalculated values in screen maps
//
void ScreenMapper::UpdateValues()
{
	for(ScreenMap &map : screenMaps) {
		map.UpdateValues();
	}
}


//
// Update precalculated area values
//
void ScreenMapper::Area::UpdateValues()
{
	minX = x - width / 2.0;
	maxX = x + width / 2.0;
	minY = y - height / 2.0;
	maxY = y + height / 2.0;
}


//
// Set screen map rotation
//
void ScreenMapper::ScreenMap::SetRotation(double angle)
{
	angle *= M_PI / 180;
	rotationMatrix[0] = cos(angle);
	rotationMatrix[1] = -sin(angle);
	rotationMatrix[2] = sin(angle);
	rotationMatrix[3] = cos(angle);
}

//
// Update precalculated screen map values
//
void ScreenMapper::ScreenMap::UpdateValues()
{
	screen.UpdateValues();
	tablet.UpdateValues();
}

