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
	TabletState filterState;
	bool filterTimedEnabled;

	timeBegin = chrono::high_resolution_clock::now();

	isRunning = true;

	//
	// Tablet input main loop
	//
	while(isRunning) {

		//
		// Read tablet position
		//
		status = tablet->ReadPosition();

		// Position OK
		if(status == Tablet::ReportValid) {
			isResent = false;
		}

		// Invalid report id
		else if(status == Tablet::ReportInvalid) {
			tablet->state.isValid = false;
			continue;
		}

		// Valid report but position is not in-range or invalid
		else if(status == Tablet::ReportPositionInvalid) {
			if(!isResent && tablet->state.isValid) {
				isResent = true;
				tablet->state.isValid = false;
				outputState.isValid = false;
			}
			else {
				continue;
			}
		}

		// Ignore report
		else if(status == Tablet::ReportIgnore) {
			continue;
		}

		// Reading failed
		else {
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
			LOG_DEBUG("InputState: T=%0.3f, B=%d, X=%0.3f, Y=%0.3f, P=%0.3f V=%s\n",
				delta,
				tablet->state.buttons,
				tablet->state.position.x,
				tablet->state.position.y,
				tablet->state.pressure,
				tablet->state.isValid ? "True" : "False"
			);
		}


		// Set output values
		if(status == Tablet::ReportPositionInvalid) {
			tablet->state.buttons = 0;
		}

		//
		// Report filters
		//
		// Is there any filters?
		if(tablet->filterReportCount > 0) {

			// Copy input state values to filter state
			memcpy(&filterState, &tablet->state, sizeof(TabletState));


			// Loop through filters
			for(int filterIndex = 0; filterIndex < tablet->filterReportCount; filterIndex++) {

				// Filter
				filter = tablet->filterReport[filterIndex];

				// Enabled?
				if(filter != NULL && filter->isEnabled) {

					// Process
					filter->SetTarget(&filterState);
					filter->Update();
					filter->GetOutput(&filterState);
				}

			}

			lock.lock();
			memcpy(&outputState, &filterState, sizeof(TabletState));
			lock.unlock();
		}

		// No filters
		else {

			// Copy input state values to output state
			lock.lock();
			memcpy(&outputState, &tablet->state, sizeof(TabletState));
			lock.unlock();

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

		// Write output state
		WriteOutputState(&outputState);
	}

	isRunning = false;

}

//
// Timer tick
//
void TabletHandler::OnTimerTick() {
	if(tablet == NULL) return;
	TabletFilter *filter;
	TabletState filterState;
	bool filtersEnabled = false;

	// Copy current input state values
	lock.lock();
	memcpy(&filterState, &outputState, sizeof(TabletState));
	lock.unlock();

	// Set position
	if(!filterState.isValid) {
		return;
	}

	// Loop through filters
	for(int filterIndex = 0; filterIndex < tablet->filterTimedCount; filterIndex++) {

		// Filter
		filter = tablet->filterTimed[filterIndex];

		// Filter enabled?
		if(filter->isEnabled) {
			filtersEnabled = true;
		}
		else {
			continue;
		}

		// Set filter targets
		filter->SetTarget(&filterState);

		// Update filter position
		filter->Update();

		// Set output vector
		filter->GetOutput(&filterState);

	}

	// Do not write to output if no filters are enabled
	if(!filtersEnabled) {
		return;
	}

	// Write output state
	WriteOutputState(&filterState);

}

//
// Write output state with output manager
//
void TabletHandler::WriteOutputState(TabletState * outputState)
{
	bool result;
	result = outputManager->Set(outputState);
	if(result) {
		result = outputManager->Write();
	}

	// Debug message
	if(result && logger.debugEnabled) {
		double delta = (chrono::high_resolution_clock::now() - timeBegin).count() / 1000000.0;
		LOG_DEBUG("OutputState: T=%0.3f, B=%d, X=%0.3f, Y=%0.3f, P=%0.3f V=%s\n",
			delta,
			outputState->buttons,
			outputState->position.x,
			outputState->position.y,
			outputState->pressure,
			outputState->isValid ? "True" : "False"
		);
	}

}
