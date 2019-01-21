#include "precompiled.h"
#include "TabletFilter.h"


//
// Constructor
//
TabletFilter::TabletFilter() {
	timerInterval = 10;
	isValid = false;
	isEnabled = false;
}


//
// Set filter output position
//
void TabletFilter::SetOutput(TabletState * tabletState) {
	memcpy(&outputState, tabletState, sizeof(TabletState));
}

//
// Get filter output position
//
bool TabletFilter::GetOutput(TabletState *tabletState) {
	memcpy(tabletState, &outputState, sizeof(TabletState));
	return true;
}

void TabletFilter::OnTimerIntervalChange(double oldInterval, double newInterval) {
	timerInterval = newInterval;
}

