#include "stdafx.h"
#include "TabletHandler.h"

#define LOG_MODULE "TabletHandler"
#include "Logger.h"

//
// Constructor
//
TabletHandler::TabletHandler() {
	tablet = NULL;
	tabletInputThread = NULL;
	isRunning = false;
	timerInterval = 10;
}


//
// Destructor
//
TabletHandler::~TabletHandler() {
	StopTimer();
}


//
// Start tablet handler
//
bool TabletHandler::Start() {
	if(tablet == NULL) return false;
	ChangeTimerInterval((int)round(timerInterval));
	tabletInputThread = new thread(&TabletHandler::RunTabletInputThread, this);
	return true;
}

//
// Stop tablet handler
//
bool TabletHandler::Stop() {
	if(tablet == NULL) return false;
	isRunning = false;
	return true;
}


//
// Start filter timer
//
bool TabletHandler::StartTimer() {
	BOOL result = CreateTimerQueueTimer(
		&timer,
		NULL, TimerCallback,
		this,
		0,
		(int)timerInterval,
		WT_EXECUTEDEFAULT
	);
	if(!result) return false;

	return true;
}


//
// Stop filter timer
//
bool TabletHandler::StopTimer() {
	if(timer == NULL) return true;
	bool result = DeleteTimerQueueTimer(NULL, timer, NULL);
	if(result) {
		timer = NULL;
		return true;
	}
	return false;
}

//
// Change timer interval 
//
void TabletHandler::ChangeTimerInterval(int newInterval) {

	double oldInterval = timerInterval;
	timerInterval = newInterval;
	
	// Tell the new interval to timed filters
	if(tablet != NULL) {
		for(int i = 0; i < tablet->filterTimedCount; i++) {
			tablet->filterTimed[i]->OnTimerIntervalChange(oldInterval, newInterval);
		}
	}

	if(StopTimer()) {
		StartTimer();
	}
}

//
// Tablet input thread
//
void TabletHandler::RunTabletInputThread() {
	int status;
	bool isFirstReport = true;
	bool isResent = false;
	TabletFilter *filter;
	bool filterTimedEnabled;
	TabletState outputState;

	chrono::high_resolution_clock::time_point timeBegin = chrono::high_resolution_clock::now();

	isRunning = true;

	//
	// Main Loop
	//

	while(isRunning) {

		//
		// Read tablet position
		//
		status = tablet->ReadPosition();

		// Position OK
		if(status == Tablet::ReportValid) {
			isResent = false;

		// Invalid report id
		} else if(status == Tablet::ReportInvalid) {
			tablet->state.isValid = false;
			continue;

		// Valid report but position is not in-range or invalid
		} else if(status == Tablet::ReportPositionInvalid) {
			if(!isResent && tablet->state.isValid) {
				isResent = true;
				tablet->state.isValid = false;
			} else {
				continue;
			}

		// Ignore report
		} else if(status == Tablet::ReportIgnore) {
			continue;

		// Reading failed
		} else {
			LOG_ERROR("Tablet Read Error!\n");
			CleanupAndExit(1);
		}

		//
		// Don't send the first report
		//
		if(isFirstReport) {
			isFirstReport = false;
			continue;
		}

		// Debug messages
		if(logger.debugEnabled) {
			double delta = (tablet->state.time - timeBegin).count() / 1000000.0;
			LOG_DEBUG("TabletState: T=%0.3f, B=%d, X=%0.3f, Y=%0.3f, P=%0.3f\n",
				delta,
				tablet->state.buttons,
				tablet->state.position.x,
				tablet->state.position.y,
				tablet->state.pressure
			);
		}


		// Set output values
		if(status == Tablet::ReportPositionInvalid) {
			tablet->state.buttons = 0;
		}

		// Copy input state values to ouput state
		memcpy(&outputState, &tablet->state, sizeof(outputState));


		//
		// Report filters
		//
		// Is there any filters?
		if(tablet->filterReportCount > 0) {

			// Loop through filters
			for(int filterIndex = 0; filterIndex < tablet->filterReportCount; filterIndex++) {

				// Filter
				filter = tablet->filterReport[filterIndex];

				// Enabled?
				if(filter != NULL && filter->isEnabled) {

					// Process
					filter->SetTarget(&outputState);
					filter->Update();
					filter->GetOutput(&outputState);
				}

			}
		}


		// Timed filter enabled?
		filterTimedEnabled = false;
		for(int filterIndex = 0; filterIndex < tablet->filterTimedCount; filterIndex++) {
			if(tablet->filterTimed[filterIndex]->isEnabled)
				filterTimedEnabled = true;
		}

		// Do not write report when timed filter is enabled
		if(filterTimedEnabled) {
			continue;
		}

		outputManager->Set(&outputState);
		outputManager->Write();
	}

	isRunning = false;

}

//
// Timer tick
//
void TabletHandler::OnTimerTick() {
	if(tablet == NULL) return;

	Vector2D position;
	TabletFilter *filter;
	TabletState outputState;
	bool filterEnabled = false;

	// Set position
	if(tablet->state.isValid) {
		position.Set(tablet->state.position);
	} else {
		return;
	}

	// Copy input state values to ouput state
	memcpy(&outputState, &tablet->state, sizeof(TabletState));

	// Loop through filters
	for(int filterIndex = 0; filterIndex < tablet->filterTimedCount; filterIndex++) {

		// Filter
		filter = tablet->filterTimed[filterIndex];

		// Filter enabled?
		if(filter->isEnabled) {
			filterEnabled = true;
		} else {
			continue;
		}

		// Set filter targets
		filter->SetTarget(&outputState);

		// Update filter position
		filter->Update();

		// Set output vector
		filter->GetOutput(&outputState);

	}

	if(!filterEnabled) {
		return;
	}

	outputManager->Set(&outputState);
	outputManager->Write();

}
