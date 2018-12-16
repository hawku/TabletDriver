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
	timerInterval = 10;
}


//
// Destructor
//
TabletHandler::~TabletHandler() {
	StopTimer();
	isRunning = false;

	if(tablet != NULL) {
		if(tablet->hidDevice != NULL)
			tablet->hidDevice->CloseDevice();

		if(tablet->hidDeviceAux != NULL)
			tablet->hidDeviceAux->CloseDevice();

		if(tablet->usbDevice != NULL)
			tablet->usbDevice->CloseDevice();
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
				bool isDown = buttons & (1 << buttonIndex);
				bool isPressed = ((buttons & (1 << buttonIndex)) > 0 && (lastButtons & (1 << buttonIndex)) == 0);
				bool isReleased = ((buttons & (1 << buttonIndex)) == 0 && (lastButtons & (1 << buttonIndex)) > 0);

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
							if(isDown) {

								// Get rotated pen position
								Vector2D scrollPosition;
								scrollPosition.Set(tablet->state.position);
								mapper->GetRotatedTabletPosition(&scrollPosition.x, &scrollPosition.y);

								// Reset last scroll position and set the scroll start position
								if(isPressed) {
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
// Auxiliary input thread (tablet buttons, etc.)
//
void TabletHandler::RunAuxInputThread()
{
	int reportStatus;
	USHORT buttons = 0;
	USHORT lastButtons = 0;
	Tablet::TabletAuxState auxState;

	// No aux device or not aux report id set
	if(tablet->hidDeviceAux == NULL && tablet->usbDevice == NULL && tablet->settings.auxReportId <= 0) {
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
