
#include "precompiled.h"
#include "TabletHandler.h"

#define LOG_MODULE "TabletHandler"
#include "Logger.h"

extern "C" NTSYSAPI NTSTATUS NTAPI NtSetTimerResolution(ULONG DesiredResolution, BOOLEAN SetResolution, PULONG CurrentResolution);
extern "C" NTSYSAPI NTSTATUS NTAPI NtQueryTimerResolution(OUT PULONG MinimumResolution, OUT PULONG MaximumResolution, OUT PULONG ActualResolution);


//
// Constructor
//
TabletHandler::TabletHandler() {
	tablet = NULL;
	tabletInputThread = NULL;
	auxInputThread = NULL;
	SetRunningState(false);
	timerInterval = 20;
	isTimerTickRunning = false;
	_isTimerStopping = false;
}


//
// Destructor
//
TabletHandler::~TabletHandler() {

	// Set running state
	SetRunningState(false);

	// Timer
	lockTimer.lock();
	StopTimer();
	lockTimer.unlock();

	// Close tablet devices
	if (tablet != NULL) {
		tablet->CloseDevice();
	}

	// Wait tablet input thread to exit
	if (tabletInputThread != NULL) {
		printf("Join tablet thread\n");
		try {
			tabletInputThread->join();
		}
		catch (std::exception &e) {
			printf("Tablet input thread exception: %s\n", e.what());
		}
	}

	// Wait auxiliary input thread to exit
	if (auxInputThread != NULL) {

		// Notify aux state change
		tablet->conditionAuxState.notify_all();

		printf("Join aux thread\n");
		try {
			auxInputThread->join();
		}
		catch (std::exception &e) {
			printf("Aux input thread exception: %s\n", e.what());
		}
	}

}


//
// Start tablet handler
//
bool TabletHandler::Start() {
	if (tablet == NULL) return false;

	// Set input emulator
	tablet->settings.SetInputEmulator(&tabletHandler->inputEmulator);

	SetRunningState(true);

	// Timer
	ChangeTimerInterval((int)round(timerInterval));

	// Threads
	tabletInputThread = new std::thread(&TabletHandler::RunTabletInputThread, this);
	auxInputThread = new std::thread(&TabletHandler::RunAuxInputThread, this);

	return true;
}

//
// Stop tablet handler
//
bool TabletHandler::Stop() {
	if (tablet == NULL) return false;
	SetRunningState(false);
	return true;
}


//
// Start filter timer
//
bool TabletHandler::StartTimer() {

	isTimerTickRunning = false;
	_isTimerStopping = false;

	if (timer == NULL) {
		BOOL result = CreateTimerQueueTimer(
			&timer,
			NULL,
			TimerCallback,
			this,
			0,
			(int)timerInterval,
			WT_EXECUTEDEFAULT
		);

		if (!result) return false;
	}
	else {
		return false;
	}

	return true;
}


//
// Change timer interval
//
bool TabletHandler::ChangeTimer(int interval) {

	if (timer != NULL) {
		BOOL result = ChangeTimerQueueTimer(NULL, timer, 0, interval);
		if (!result) return false;
		return true;
	}

	return false;
}


//
// Stop filter timer
//
bool TabletHandler::StopTimer() {
	if (timer == NULL) return true;
	if (_isTimerStopping) return false;

	_isTimerStopping = true;

	// Wait timer tick to finish
	for (int i = 0; i < 100 + timerInterval; i++) {
		if (!isTimerTickRunning) break;
		Sleep(1);
	}
	if (isTimerTickRunning) {
		LOG_ERROR("Timer did not stop!\n");
		return false;
	}

	bool result = DeleteTimerQueueTimer(NULL, timer, NULL);
	if (result) {
		timer = NULL;
	}
	return true;
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
	if (logger.IsDebugOutputEnabled()) {
		LOG_DEBUG("System timer resolution:\n");
		LOG_DEBUG("  Minimum: %0.3f ms\n", (double)timerMinimumResolution / 10000.0);
		LOG_DEBUG("  Maximum: %0.3f ms\n", (double)timerMaximumResolution / 10000.0);
		LOG_DEBUG("  Current: %0.3f ms\n", (double)timerCurrentResolution / 10000.0);
	}

	// Check if current resolution is double the timer interval
	if (timerCurrentResolution > timerInterval * 5000.0)
	{
		ULONG timerResolution = (ULONG)(timerInterval * 5000.0);
		ULONG currentResolution;

		// Limits
		if (timerResolution > timerMinimumResolution) {
			timerResolution = timerMinimumResolution;
		}
		else if (timerResolution < timerMaximumResolution) {
			timerResolution = timerMaximumResolution;
		}

		// Set new timer resolution
		NtSetTimerResolution(timerResolution, true, &currentResolution);

		// Get current resoltution
		timerCurrentResolution = 0;
		NtQueryTimerResolution(&timerMinimumResolution, &timerMaximumResolution, &timerCurrentResolution);

		// Debug messages
		if (logger.IsDebugOutputEnabled()) {
			LOG_DEBUG("System timer resolution set to %0.3f ms\n", (double)timerResolution / 10000.0);
			LOG_DEBUG("System timer resolution is now %0.3f ms\n", (double)timerCurrentResolution / 10000.0);
		}
	}

	// Tell the new interval to timed filters
	filtersEnabled = false;
	if (tablet != NULL) {
		for (int i = 0; i < tablet->filterTimedCount; i++) {
			tablet->filterTimed[i]->OnTimerIntervalChange(oldInterval, newInterval);

			// Filter enabled?
			if (tablet->filterTimed[i]->isEnabled) {
				filtersEnabled = true;
			}
		}
	}

	// Lock timer
	lockTimer.lock();

	// Stop the timer when filters are disabled and timer is running
	if (!filtersEnabled && timer != NULL) {
		StopTimer();
	}

	// Start the timer when filters are enabled and timer is not running
	else if (filtersEnabled && timer == NULL) {
		StartTimer();
	}

	// Start timer interval when filters are enabled and timer is running
	else if (filtersEnabled) {
		ChangeTimer((int)newInterval);
	}

	// Unlock timer
	lockTimer.unlock();
}


//
// Button helper functions
//
bool TabletHandler::IsButtonDown(UINT32 buttons, int buttonIndex) {
	return buttons & (1 << buttonIndex);
}
bool TabletHandler::IsButtonPressed(UINT32 buttons, UINT32 lastButtons, int buttonIndex) {
	return ((buttons & (1 << buttonIndex)) > 0 && (lastButtons & (1 << buttonIndex)) == 0);
}
bool TabletHandler::IsButtonReleased(UINT32 buttons, UINT32 lastButtons, int buttonIndex) {
	return ((buttons & (1 << buttonIndex)) == 0 && (lastButtons & (1 << buttonIndex)) > 0);
}



//
// Process pen and auxiliary buttons
//
void TabletHandler::ProcessPenButtons(UINT32 *outButtons) {
	ProcessButtons(outButtons, true);
}
void TabletHandler::ProcessAuxButtons(UINT32 *outButtons) {
	ProcessButtons(outButtons, false);
}
void TabletHandler::ProcessButtons(UINT32 *outButtons, bool isPen)
{
	UINT32 penButtons = 0;
	UINT32 penLastButtons = 0;
	UINT32 buttons;
	UINT32 lastButtons;
	int buttonCount;
	bool isDown;
	bool isPressed;
	bool isReleased;
	bool hasBinding;
	std::string key;
	InputEmulator::InputActionCollection *inputCollection = NULL;
	bool scrolled = false;
	Vector2D scrollPosition;

	// Pen buttons
	penButtons = tablet->state.inputButtons;
	penLastButtons = tablet->state.lastButtons;
	if (isPen) {
		buttons = penButtons;
		lastButtons = penLastButtons;
		buttonCount = tablet->settings.buttonCount;
	}

	// Auxiliary buttons
	else {
		buttons = tablet->auxState.buttons;
		lastButtons = tablet->auxState.lastButtons;
		buttonCount = tablet->settings.auxButtonCount;
	}

	// Button state changed?
	if (buttons > 0 || lastButtons > 0) {

		// Loop through buttons
		for (int buttonIndex = 0; buttonIndex < buttonCount; buttonIndex++) {

			// Button is not down, pressed or released?
			if ((buttons & (1 << buttonIndex)) == 0 && (lastButtons & (1 << buttonIndex)) == 0) {
				continue;
			}

			// Button state
			isDown = IsButtonDown(buttons, buttonIndex);
			isPressed = IsButtonPressed(buttons, lastButtons, buttonIndex);
			isReleased = IsButtonReleased(buttons, lastButtons, buttonIndex);

			// Pen button map
			hasBinding = false;
			inputCollection = NULL;
			if (isPen && tablet->settings.buttonMap[buttonIndex].Count() > 0) {
				inputCollection = &tablet->settings.buttonMap[buttonIndex];
				hasBinding = true;
			}

			// Auxiliary button map
			else if (!isPen && tablet->settings.auxButtonMap[buttonIndex].Count() > 0) {
				inputCollection = &tablet->settings.auxButtonMap[buttonIndex];
				hasBinding = true;
			}


			// No input actions found -> skip
			if (inputCollection == NULL) {
				continue;
			}

			int indexStart = 0;
			int indexAdd = 1;
			int length = inputCollection->Count();

			// Execute in reverse order when button is released
			if (isReleased) {
				indexStart = inputCollection->Count() - 1;
				indexAdd = -1;
			}

			// Loop through input actions
			int index = indexStart;
			for(int i = 0; i < length; i++) {

				InputEmulator::InputAction *inputAction = inputCollection->actions[index];
				index += indexAdd;

				int mouseButton = inputAction->mouseButton;

				//
				// Mouse buttons
				//
				if (inputAction->type == InputEmulator::ActionTypeMouse &&
					mouseButton >= InputEmulator::Mouse1 && mouseButton <= InputEmulator::Mouse5
					) {

					// Pen button
					if (isPen && isDown) {
						*outButtons |= (1 << (mouseButton - 1));
					}

					// Auxiliary button
					else if (!isPen) {
						inputAction->Execute(isPressed, isReleased, isDown);
					}
				}

				//
				// Mouse scroll and volume control
				//
				else if (
					(
						inputAction->type == InputEmulator::ActionTypeMouse &&
						(
							mouseButton == InputEmulator::MouseScrollVertical
							||
							mouseButton == InputEmulator::MouseScrollHorizontal
							||
							mouseButton == InputEmulator::MouseScrollBoth
							)
						)
					||
					inputAction->type == InputEmulator::ActionTypeAudioVolumeControl
					||
					inputAction->type == InputEmulator::ActionTypeAudioBalanceControl
					) {

					// Scroll button down?
					if (isDown
						&&
						// Scroll when tip is down?
						(!tablet->settings.scrollDrag || IsButtonDown(penButtons, 0))
						) {

						// Get rotated pen position
						scrollPosition.Set(tablet->state.position);
						mapper->GetRotatedTabletPosition(&scrollPosition.x, &scrollPosition.y);

						// Reset last scroll position and set the scroll start position
						if (
							isPressed
							||
							(tablet->settings.scrollDrag && IsButtonPressed(penButtons, penLastButtons, 0))
							) {
							lastScrollPosition.Set(scrollPosition);
							scrollStartPosition.Set(tablet->state.position);

							//
							// Move normal mouse to digitizer position
							//
							if (outputManager->mode == OutputManager::ModeVMultiDigitizer) {
								TabletState tmpState;
								tmpState.position.Set(tablet->state.position);
								outputManager->sendInputAbsolute.Set(&tmpState);
								outputManager->sendInputAbsolute.Write();
							}

						}

						// Disable mouse tip button when using drag scroll
						if (tablet->settings.scrollDrag) {
							*outButtons &= ~1;
						}

						// Delta from the last scroll position
						Vector2D delta(
							(scrollPosition.x - lastScrollPosition.x) * tablet->settings.scrollSensitivity,
							(scrollPosition.y - lastScrollPosition.y) * tablet->settings.scrollSensitivity
						);

						// X Acceleration
						if (delta.x > 0)
							delta.x = round(pow(delta.x, tablet->settings.scrollAcceleration));
						else
							delta.x = -round(pow(-delta.x, tablet->settings.scrollAcceleration));

						// Y Acceleration
						if (delta.y > 0)
							delta.y = round(pow(delta.y, tablet->settings.scrollAcceleration));
						else
							delta.y = -round(pow(-delta.y, tablet->settings.scrollAcceleration));


						// Vertical Scroll
						if (delta.y != 0 && (
							mouseButton == InputEmulator::MouseScrollVertical
							||
							mouseButton == InputEmulator::MouseScrollBoth
							)) {
							inputEmulator.MouseScroll((int)delta.y, true);
							scrolled = true;
						}

						// Horizontal scroll
						if (delta.x != 0 && (
							mouseButton == InputEmulator::MouseScrollHorizontal
							||
							mouseButton == InputEmulator::MouseScrollBoth
							)) {
							inputEmulator.MouseScroll((int)-delta.x, false);
							scrolled = true;
						}

						// Media volume control
						if (delta.y != 0 && inputAction->type == InputEmulator::ActionTypeAudioVolumeControl) {
							inputEmulator.VolumeChange(-(float)delta.y / 100.0f);
							scrolled = true;
						}

						// Media balance control
						if ((delta.x != 0 || isPressed) && inputAction->type == InputEmulator::ActionTypeAudioBalanceControl) {
							double balance = 0.5 + ((scrollStartPosition.x - scrollPosition.x) * tablet->settings.scrollSensitivity) / 50.0f;
							inputEmulator.VolumeBalance((float)balance);
							scrolled = true;
						}

						// Stop cursor
						if (tablet->settings.scrollStopCursor) {
							tablet->state.position.Set(scrollStartPosition);
						}

					}

				}

				//
				// All other actions
				//
				else {
					inputAction->Execute(isPressed, isReleased, isDown);
				}

			}

		}

		// Update last scroll position
		if (scrolled) {
			lastScrollPosition.Set(scrollPosition);
		}

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
	TabletState oldState;
	bool filterTimedEnabled;
	UINT32 buttons = 0;
	UINT32 lastButtons = 0;
	UINT32 outButtons = 0;

	timeBegin = std::chrono::high_resolution_clock::now();

	//
	// Tablet input main loop
	//
	while (IsRunning()) {

		//
		// Read tablet position
		//
		status = tablet->ReadState();

		if (!tablet->isOpen) break;

		// Position OK
		if (status == Tablet::ReportValid) {
			isResent = false;
		}

		// Invalid report id
		else if (status == Tablet::ReportInvalid) {
			tablet->state.isValid = false;
			continue;
		}

		// Valid report but position is not in-range or invalid
		else if (status == Tablet::ReportPositionInvalid) {
			if (!isResent && tablet->state.isValid) {
				isResent = true;
				tablet->state.isValid = false;
				outputState.isValid = false;
			}
			else {
				continue;
			}
		}

		// Ignore report
		else if (status == Tablet::ReportIgnore) {
			continue;
		}

		// Reading failed
		else {
			LOG_ERROR("Tablet Read Error!\n");
			CleanupAndExit(0);
			return;
		}


		//
		// Don't use first report
		//
		if (isFirstReport) {
			isFirstReport = false;
			continue;
		}

		//
		// Velocity calculation
		//
		if (oldState.isValid) {
			double timeDelta = (tablet->state.time - oldState.time).count() / 1000000.0;
			if (timeDelta >= 1 && timeDelta <= 10) {
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
		if (logger.IsDebugOutputEnabled()) {
			double delta = (tablet->state.time - timeBegin).count() / 1000000.0;
			LOG_DEBUG("InputState: T=%0.3f, B=%d, X=%0.3f, Y=%0.3f, P=%0.3f H=%0.2f V=%0.2f Valid=%s\n",
				delta,
				tablet->state.inputButtons,
				tablet->state.inputPosition.x,
				tablet->state.inputPosition.y,
				tablet->state.inputPressure,
				tablet->state.inputHeight,
				tablet->state.inputVelocity,
				tablet->state.isValid ? "True" : "False"
			);
		}

		// Set output values
		if (status == Tablet::ReportPositionInvalid) {
			tablet->state.buttons = 0;
		}

		// Process buttons
		outButtons = 0;
		ProcessPenButtons(&outButtons);
		ProcessAuxButtons(&outButtons);
		tablet->state.lastButtons = tablet->state.inputButtons;
		tablet->state.buttons = outButtons;

		//
		// Report filters
		//
		// Are there any filters?
		if (tablet->filterReportCount > 0) {

			// Copy input state values to filter state
			memcpy(&filterState, &tablet->state, sizeof(TabletState));


			// Loop through filters
			for (int filterIndex = 0; filterIndex < tablet->filterReportCount; filterIndex++) {

				// Filter
				filter = tablet->filterReport[filterIndex];

				// Enabled?
				if (filter != NULL && filter->isEnabled) {

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
		for (int filterIndex = 0; filterIndex < tablet->filterTimedCount; filterIndex++) {
			if (tablet->filterTimed[filterIndex]->isEnabled)
				filterTimedEnabled = true;
		}


		// Set old state
		memcpy(&oldState, &tablet->state, sizeof(TabletState));

		// Do not write report when timed filter is enabled
		if (filterTimedEnabled) {
			continue;
		}

		// Write output state
		memcpy(&outputStateWrite, &outputState, sizeof(TabletState));
		WriteOutputState(&outputStateWrite);
	}

	SetRunningState(false);

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
	if (tablet->hidDeviceAux == NULL && tablet->usbDevice == NULL && tablet->settings.auxReports[0].reportId <= 0) {
		return;
	}

	//
	// Auxiliary input main loop
	//
	while (IsRunning()) {

		// Read
		reportStatus = tablet->ReadAuxReport();

		// Read error
		if (reportStatus == Tablet::AuxReportReadError) {
			LOG_ERROR("Auxiliary device read error!\n");
			break;
		}

		// Aux state invalid or ignored
		if (reportStatus == Tablet::AuxReportInvalid || reportStatus == Tablet::AuxReportIgnore) {
			continue;
		}

		// Skip invalid state
		if (!tablet->auxState.isValid) continue;
		memcpy(&auxState, &tablet->auxState, sizeof(Tablet::TabletAuxState));
		tablet->auxState.isValid = false;

		// Process buttons
		if (logger.IsDebugOutputEnabled()) {
			LOG_DEBUG("Aux buttons: 0x%04X\n", auxState.buttons);
		}
		UINT32 tmpButtons = 0;
		ProcessAuxButtons(&tmpButtons);
		tablet->auxState.lastButtons = tablet->auxState.buttons;

	}

}


//
// Timer tick
//
void TabletHandler::OnTimerTick() {
	if (tablet == NULL) return;
	if (timer == NULL) return;
	if (_isTimerStopping) return;

	TabletFilter *filter;
	TabletState filterState;
	bool filtersEnabled = false;

	// Detect performance problems
	if (isTimerTickRunning) {

		// Limit error logging rate
		double timeDelta = (std::chrono::high_resolution_clock::now() - timeLastTimerProblem).count() / 1000000.0;
		if (timeDelta > 2000.0) {
			LOG_WARNING("Filter performance problem detected! Use lower filter rate or disable the smoothing filter.\n");
			timeLastTimerProblem = std::chrono::high_resolution_clock::now();
		}
		return;
	}

	isTimerTickRunning = true;

	// Loop through filters
	for (int filterIndex = 0; filterIndex < tablet->filterTimedCount; filterIndex++) {

		// Filter
		filter = tablet->filterTimed[filterIndex];

		// Filter enabled?
		if (filter->isEnabled) {

			// Copy current input state values when a enabled filter is found
			if (!filtersEnabled) {
				lock.lock();
				memcpy(&filterState, &outputState, sizeof(TabletState));
				lock.unlock();

				// State valid?
				if (!filterState.isValid) {
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
	if (!filtersEnabled) {
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
	if (result) {
		result = outputManager->Write();
	}

	// Debug message
	if (result && logger.IsDebugOutputEnabled()) {
		double delta = (std::chrono::high_resolution_clock::now() - timeBegin).count() / 1000000.0;
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
