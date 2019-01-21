#include "precompiled.h"
#include "TabletMeasurement.h"

#define LOG_MODULE "Measurement"
#include "Logger.h"

//
// Constructor
//
TabletMeasurement::TabletMeasurement() {
	minimum.Set(0, 0);
	maximum.Set(0, 0);
	pointCount = 0;
	totalReports = 0;
	reportCounter = 0;
	lastState.buttons = 0;
	lastState.isValid = false;
}

//
// Destructor
//
TabletMeasurement::~TabletMeasurement() {
}


//
// Start measurement
//
void TabletMeasurement::Start() {
	Start(-1);
}

//
// Start measurement
//
void TabletMeasurement::Start(int reportCount) {

	// Limits
	minimum.Set(10000, 10000);
	maximum.Set(-10000, -10000);

	pointCount = 0;
	totalReports = reportCount;
	reportCounter = reportCount;

	// Last tablet state
	lastState.buttons = 0;
	lastState.isValid = false;

	lastPointTime = 0;

	SetRunningState(true);
}

//
// Stop measurement
//
void TabletMeasurement::Stop() {
	SetRunningState(false);
}


//
// Update measurement 
//
void TabletMeasurement::Update(TabletState state) {

	DWORD time;


	if(IsRunning()) {
		if(reportCounter > 0 || reportCounter <= -1) {

			// X Limits
			if(state.position.x < minimum.x) minimum.x = state.position.x;
			else if(state.position.x > maximum.x) maximum.x = state.position.x;

			// Y Limits
			if(state.position.y < minimum.y) minimum.y = state.position.y;
			else if(state.position.y > maximum.y) maximum.y = state.position.y;

			// Detect pen tip release
			if((state.buttons & 0x01) == 0 && lastState.isValid && (lastState.buttons & 0x01) == 1 && pointCount < 10) {

				time = GetTickCount();

				// Pen tip debounce (100 ms)
				if(time - lastPointTime > 100) {
					lastPointTime = time;
					points[pointCount].Set(state.position);
					pointCount++;
				}
			}

			lastState.buttons = state.buttons;
			lastState.isValid = true;
			reportCounter--;
		}
		else {
			SetRunningState(false);
		}
	}
}