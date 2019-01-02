
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
	auxInputThread = NULL;
	isRunning = false;
	timerInterval = 20;
	isTimerTickRunning = false;
}


//
// Destructor
//
TabletHandler::~TabletHandler() {
	StopTimer();
	isRunning = false;
	if(tablet != NULL) {
		tablet->isOpen = false;
	}
}


//
// Start tablet handler
//
bool TabletHandler::Start() {
	if(tablet == NULL) return false;
	ChangeTimerInterval((int)round(timerInterval));
	isRunning = true;
	tabletInputThread = new thread(&TabletHandler::RunTabletInputThread, this);
	auxInputThread = new thread(&TabletHandler::RunAuxInputThread, this);
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
	bool filtersEnabled;

	ULONG timerMinimumResolution = 0;
	ULONG timerMaximumResolution = 0;
	ULONG timerCurrentResolution = 0;
	NtQueryTimerResolution(&timerMinimumResolution, &timerMaximumResolution, &timerCurrentResolution);

	// Debug messages
	if(logger.debugEnabled) {
		LOG_DEBUG("System timer resolution:\n");
		LOG_DEBUG("  Minimum: %0.3f ms\n", (double)timerMinimumResolution / 10000.0);
		LOG_DEBUG("  Maximum: %0.3f ms\n", (double)timerMaximumResolution / 10000.0);
		LOG_DEBUG("  Current: %0.3f ms\n", (double)timerCurrentResolution / 10000.0);
	}

	// Check if current resolution is double the timer interval
	if(timerCurrentResolution > timerInterval * 5000.0)
	{
		ULONG timerResolution = (ULONG)(timerInterval * 5000.0);
		ULONG currentResolution;

		// Limits
		if(timerResolution > timerMinimumResolution) {
			timerResolution = timerMinimumResolution;
		}
		else if(timerResolution < timerMaximumResolution) {
			timerResolution = timerMaximumResolution;
		}

		// Set new timer resolution
		NtSetTimerResolution(timerResolution, true, &currentResolution);

		// Get current resoltution
		timerCurrentResolution = 0;
		NtQueryTimerResolution(&timerMinimumResolution, &timerMaximumResolution, &timerCurrentResolution);

		// Debug messages
		if(logger.debugEnabled) {
			LOG_DEBUG("System timer resolution set to %0.3f ms\n", (double)timerResolution / 10000.0);
			LOG_DEBUG("System timer resolution is now %0.3f ms\n", (double)timerCurrentResolution / 10000.0);
		}
	}

	// Tell the new interval to timed filters
	filtersEnabled = false;
	if(tablet != NULL) {
		for(int i = 0; i < tablet->filterTimedCount; i++) {
			tablet->filterTimed[i]->OnTimerIntervalChange(oldInterval, newInterval);

			// Filter enabled?
			if(tablet->filterTimed[i]->isEnabled) {
				filtersEnabled = true;
			}
		}
	}

	// Stop the timer and restart if a filter is enabled
	if(StopTimer() && filtersEnabled) {
		StartTimer();
	}
}


//
// Button helper functions
//
bool IsButtonDown(UCHAR buttons, int buttonIndex) {
	return buttons & (1 << buttonIndex);
}
bool IsButtonPressed(UCHAR buttons, UCHAR lastButtons, int buttonIndex) {
	return ((buttons & (1 << buttonIndex)) > 0 && (lastButtons & (1 << buttonIndex)) == 0);
}
bool IsButtonReleased(UCHAR buttons, UCHAR lastButtons, int buttonIndex) {
	return ((buttons & (1 << buttonIndex)) == 0 && (lastButtons & (1 << buttonIndex)) > 0);
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
	TabletState oldState;
	bool filterTimedEnabled;
	UCHAR buttons = 0;
	UCHAR outButtons = 0;
	UCHAR lastButtons = 0;
	Vector2D lastScrollPosition;
	Vector2D scrollStartPosition;

	timeBegin = chrono::high_resolution_clock::now();

	//
	// Tablet input main loop
	//
	while(isRunning) {

		//
		// Read tablet position
		//
		status = tablet->ReadState();

		if(!tablet->isOpen) break;

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
		// Don't use first report
		//
		if(isFirstReport) {
			isFirstReport = false;
			continue;
		}

		//
		// Velocity calculation
		//
		if(oldState.isValid) {
			double timeDelta = (tablet->state.time - oldState.time).count() / 1000000.0;
			if(timeDelta >= 1 && timeDelta <= 10) {
				tablet->state.inputVelocity = oldState.inputPosition.Distance(tablet->state.inputPosition) * (1000.0 / timeDelta);
			}
			else {
				tablet->state.inputVelocity = oldState.inputVelocity;
			}
		}
		else {
			tablet->state.inputVelocity = oldState.inputVelocity;
		}

		// Debug messages
		if(logger.debugEnabled) {
			double delta = (tablet->state.time - timeBegin).count() / 1000000.0;
			LOG_DEBUG("InputState: T=%0.3f, B=%d, X=%0.3f, Y=%0.3f, P=%0.3f V=%0.2f Valid=%s\n",
				delta,
				tablet->state.inputButtons,
				tablet->state.inputPosition.x,
				tablet->state.inputPosition.y,
				tablet->state.inputPressure,
				tablet->state.inputVelocity,
				tablet->state.isValid ? "True" : "False"
			);
		}

		// Set output values
		if(status == Tablet::ReportPositionInvalid) {
			tablet->state.buttons = 0;
		}

		//
		// Button map
		//
		buttons = tablet->state.buttons;
		outButtons = 0;

		if(buttons > 0 || lastButtons > 0) {

			// Loop through buttons
			for(int buttonIndex = 0; buttonIndex < tablet->settings.buttonCount; buttonIndex++) {

				// Button is not down, pressed or released?
				if((buttons & (1 << buttonIndex)) == 0 && (lastButtons & (1 << buttonIndex)) == 0) {
					continue;
				}

				// Button state
				bool isDown = IsButtonDown(buttons, buttonIndex);
				bool isPressed = IsButtonPressed(buttons, lastButtons, buttonIndex);
				bool isReleased = IsButtonReleased(buttons, lastButtons, buttonIndex);

				// Button is set
				if(tablet->settings.buttonMap[buttonIndex].size() > 0) {

					string key = tablet->settings.buttonMap[buttonIndex];

					if(inputEmulator.keyMap.count(key) > 0) {
						InputEmulator::KeyMapValue *keyMapValue = inputEmulator.keyMap[key];

						// Mouse buttons
						if(keyMapValue->mouseButton > 0 && keyMapValue->mouseButton < 8) {
							if(isDown) {
								outButtons |= (1 << (keyMapValue->mouseButton - 1));
							}
						}

						//
						// Mouse scroll
						//
						else if((keyMapValue->mouseButton & 0x103) > 0) {

							// Scroll pen button down?
							if(
								isDown
								&&

								// Scroll when tip is down
								(!tablet->settings.scrollDrag || IsButtonDown(buttons, 0))
								) {

								// Get rotated pen position
								Vector2D scrollPosition;
								scrollPosition.Set(tablet->state.position);
								mapper->GetRotatedTabletPosition(&scrollPosition.x, &scrollPosition.y);

								// Reset last scroll position and set the scroll start position
								if(isPressed
									||
									(tablet->settings.scrollDrag  && isDown && IsButtonPressed(buttons, lastButtons, 0))
									) {
									lastScrollPosition.Set(scrollPosition);
									scrollStartPosition.Set(tablet->state.position);

									//
									// Move normal mouse to digitizer position
									//
									if(outputManager->mode == OutputManager::ModeVMultiDigitizer) {
										TabletState tmpState;
										tmpState.position.Set(tablet->state.position);
										outputManager->sendInputAbsolute.Set(&tmpState);
										outputManager->sendInputAbsolute.Write();
									}

								}

								// Disable mouse tip button when using drag scroll
								if(tablet->settings.scrollDrag) {
									outButtons &= ~1;
								}

								// Delta from the last scroll position
								Vector2D delta(
									(scrollPosition.x - lastScrollPosition.x) * tablet->settings.scrollSensitivity,
									(scrollPosition.y - lastScrollPosition.y) * tablet->settings.scrollSensitivity
								);

								// X Acceleration
								if(delta.x > 0)
									delta.x = round(pow(delta.x, tablet->settings.scrollAcceleration));
								else
									delta.x = -round(pow(-delta.x, tablet->settings.scrollAcceleration));

								// Y Acceleration
								if(delta.y > 0)
									delta.y = round(pow(delta.y, tablet->settings.scrollAcceleration));
								else
									delta.y = -round(pow(-delta.y, tablet->settings.scrollAcceleration));


								// Vertical Scroll
								if(delta.y != 0 && (keyMapValue->mouseButton & 0x101) == 0x101) {
									inputEmulator.MouseScroll((int)delta.y, true);
									lastScrollPosition.Set(scrollPosition);
								}

								// Horizontal scroll
								if(delta.x != 0 && (keyMapValue->mouseButton & 0x102) == 0x102) {
									inputEmulator.MouseScroll((int)-delta.x, false);
									lastScrollPosition.Set(scrollPosition);
								}

								// Stop cursor
								if(tablet->settings.scrollStopCursor) {
									tablet->state.position.Set(scrollStartPosition);
								}

							}
						}

						// Keyboard key
						if(keyMapValue->virtualKey > 0) {
							if(isPressed) {
								inputEmulator.SetKeyState(keyMapValue->virtualKey, true);
							}
							else if(isReleased) {
								inputEmulator.SetKeyState(keyMapValue->virtualKey, false);
							}
						}

					}

					// Keyboard keys
					else {
						if(isPressed) {
							inputEmulator.SetInputStates(key, true);
						}
						else if(isReleased) {
							inputEmulator.SetInputStates(key, false);
						}
					}
				}

			}
		}

		// Set button values
		tablet->state.buttons = outButtons;
		lastButtons = buttons;


		//
		// Report filters
		//
		// Are there any filters?
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


		// Set old state
		memcpy(&oldState, &tablet->state, sizeof(TabletState));

		// Do not write report when timed filter is enabled
		if(filterTimedEnabled) {
			continue;
		}

		// Write output state
		memcpy(&outputStateWrite, &outputState, sizeof(TabletState));
		WriteOutputState(&outputStateWrite);
	}

	isRunning = false;

}

//
// Auxiliary input thread (tablet buttons, etc.)
//
void TabletHandler::RunAuxInputThread()
{
	int reportStatus;
	USHORT buttons = 0;
	USHORT lastButtons = 0;
	Tablet::TabletAuxState auxState;

	// No aux device or not aux report id set
	if(tablet->hidDeviceAux == NULL && tablet->usbDevice == NULL && tablet->settings.auxReports[0].reportId <= 0) {
		return;
	}

	//
	// Auxiliary input main loop
	//
	while(isRunning) {

		// Read
		reportStatus = tablet->ReadAuxReport();

		// Read error
		if(reportStatus == Tablet::AuxReportReadError) {
			LOG_ERROR("Auxiliary device read error!\n");
			break;
		}

		// Aux state invalid or ignored
		if(reportStatus == Tablet::AuxReportInvalid || reportStatus == Tablet::AuxReportIgnore) {
			continue;
		}

		// Skip invalid state
		if(!tablet->auxState.isValid) continue;
		memcpy(&auxState, &tablet->auxState, sizeof(Tablet::TabletAuxState));
		tablet->auxState.isValid = false;

		// Buttons
		buttons = auxState.buttons;

		if(logger.debugEnabled) {
			LOG_DEBUG("Aux buttons: 0x%04X\n", buttons);
		}

		// Loop through buttons
		for(int button = 1; button <= tablet->settings.auxButtonCount; button++) {

			int buttonMask = 1 << (button - 1);

			//
			// Button pressed
			//
			if((buttons & buttonMask) == buttonMask && (lastButtons & buttonMask) != buttonMask) {

				// Button mapped?
				if(tablet->settings.auxButtonMap[button - 1].size() > 0) {

					// Set key down
					inputEmulator.SetInputStates(tablet->settings.auxButtonMap[button - 1], true);
				}

			}

			//
			// Button released
			//
			else if((buttons & buttonMask) != buttonMask && (lastButtons & buttonMask) == buttonMask) {

				// Button mapped?
				if(tablet->settings.auxButtonMap[button - 1].size() > 0) {

					// Set key up
					inputEmulator.SetInputStates(tablet->settings.auxButtonMap[button - 1], false);
				}

			}

		}

		lastButtons = buttons;

	}

}

//
// Timer tick
//
void TabletHandler::OnTimerTick() {
	if(tablet == NULL) return;
	TabletFilter *filter;
	TabletState filterState;
	bool filtersEnabled = false;


	if(isTimerTickRunning) {

		// Limit error logging rate
		double timeDelta = (chrono::high_resolution_clock::now() - timeLastTimerProblem).count() / 1000000.0;
		if(timeDelta > 2000.0) {
			LOG_ERROR("Filter performance problem detected! Use lower filter rate or disable the smoothing filter.\n");
			timeLastTimerProblem = chrono::high_resolution_clock::now();
		}
		return;
	}
	isTimerTickRunning = true;

	// Loop through filters
	for(int filterIndex = 0; filterIndex < tablet->filterTimedCount; filterIndex++) {

		// Filter
		filter = tablet->filterTimed[filterIndex];

		// Filter enabled?
		if(filter->isEnabled) {

			// Copy current input state values when a enabled filter is found
			if(!filtersEnabled) {
				lock.lock();
				memcpy(&filterState, &outputState, sizeof(TabletState));
				lock.unlock();

				// State valid?
				if(!filterState.isValid) {
					isTimerTickRunning = false;
					return;
				}
			}

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
		isTimerTickRunning = false;
		return;
	}

	// Write output state
	memcpy(&outputStateWrite, &filterState, sizeof(TabletState));
	WriteOutputState(&outputStateWrite);

	isTimerTickRunning = false;
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
