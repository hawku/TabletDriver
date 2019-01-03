#include "stdafx.h"
#include "PipeHandler.h"

#define LOG_MODULE "Pipe"
#include "Logger.h"


//
// Constructor
//
PipeHandler::PipeHandler(string pipeName)
{
	pipeNameInput = "\\\\.\\PIPE\\" + pipeName + "Input";
	pipeNameOutput = "\\\\.\\PIPE\\" + pipeName + "Output";
	pipeNameState = "\\\\.\\PIPE\\" + pipeName + "State";

	handleInput = NULL;
	handleOutput = NULL;
	handleState = NULL;
	connectedInput = false;
	connectedOutput = false;
	connectedState = false;

	isRunning = false;
	isStateOutputEnabled = false;
}


//
// Destructor
//
PipeHandler::~PipeHandler()
{
	Stop();
}


//
// Start
//
bool PipeHandler::Start()
{
	lock.lock();
	bool running = isRunning;
	lock.unlock();

	if(!running) {
		lock.lock();
		isRunning = true;
		lock.unlock();
		threadInput = new thread(&PipeHandler::RunInputThread, this);
		threadOutput = new thread(&PipeHandler::RunOutputThread, this);
		threadState = new thread(&PipeHandler::RunStateThread, this);
		return true;
	}
	return false;
}


//
// Stop
//
bool PipeHandler::Stop() {

	lock.lock();
	bool running = isRunning;
	lock.unlock();

	if(running) {
		lock.lock();
		isRunning = true;
		lock.unlock();

		// Input
		if(handleInput != NULL) {
			if(connectedInput) {
				SAFE_CLOSE_HANDLE(handleInput);
			}
			TerminateThread(threadInput, 0);
		}

		// Output
		if(handleOutput != NULL) {
			if(connectedOutput) {
				SAFE_CLOSE_HANDLE(handleOutput);
			}
			TerminateThread(threadOutput, 0);
		}

		// State
		if(handleState != NULL) {
			if(connectedState) {
				SAFE_CLOSE_HANDLE(handleState);
			}
			TerminateThread(threadState, 0);
		}

		// Logger handle
		logger.pipeHandle = NULL;
		return true;
	}
	return false;
}


//
// Input thread
//
void PipeHandler::RunInputThread()
{
	BOOL result;
	DWORD bytesWritten = 0;
	DWORD bytesRead = 0;
	DWORD bytesAvailable = 0;
	DWORD bytesLeft = 0;
	char bufferWrite[1024];
	char bufferRead[1024];

	while(true) {

		lock.lock();
		if(!isRunning) {
			lock.unlock();
			break;
		}
		else {
			lock.unlock();
		}

		connectedInput = false;

		// Input connection
		handleInput = CreateNamedPipeA(pipeNameInput.c_str(),
			PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES,
			sizeof(bufferWrite),
			sizeof(bufferRead),
			500, // Timeout
			NULL
		);


		// Invalid handle?
		if(handleInput == INVALID_HANDLE_VALUE) {
			LOG_ERROR("Couldn't create input pipe!\n");
			break;
		}

		// Connect
		LOG_DEBUG("Connecting to input pipe!\n");
		result = ConnectNamedPipe(handleInput, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
		LOG_DEBUG("Input pipe connected: %d\n", result);

		if(!result) {
			SAFE_CLOSE_HANDLE(handleInput);
			continue;
		}

		connectedInput = true;

		// Read loop
		while(true) {
			PeekNamedPipe(handleInput, &bufferRead, sizeof(bufferRead), &bytesRead, &bytesAvailable, &bytesLeft);
			if(bytesAvailable > 0) {
				result = ReadFile(handleInput, &bufferRead, sizeof(bufferRead), &bytesRead, NULL);
				if(!result || bytesRead == 0) {
					if(GetLastError() == ERROR_BROKEN_PIPE) {
						LOG_DEBUG("Input pipe broken! %d\n", GetLastError());
						SAFE_CLOSE_HANDLE(handleInput);
						break;
					}
				}

				if(bytesRead > 0) {
					bufferRead[bytesRead] = 0;
					//LOG_DEBUG("Pipe read: %s\n", bufferRead);
					CommandLine *cmd = new CommandLine(bufferRead);
					ProcessCommand(cmd);
					delete cmd;
				}
			}
			else {
				Sleep(100);
			}
		}

	}




}


//
// Output thread
//
void PipeHandler::RunOutputThread()
{
	BOOL result;
	DWORD bytesWritten = 0;
	DWORD bytesRead = 0;
	char bufferWrite[1024];
	char bufferRead[1024];

	// Connect loop
	while(true) {

		lock.lock();
		if(!isRunning) {
			lock.unlock();
			break;
		}
		else {
			lock.unlock();
		}

		connectedOutput = false;

		// Output connection
		handleOutput = CreateNamedPipeA(pipeNameOutput.c_str(),
			PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES,
			sizeof(bufferWrite),
			sizeof(bufferRead),
			500, // Timeout
			NULL
		);

		if(handleOutput == INVALID_HANDLE_VALUE) {
			LOG_ERROR("Couldn't create output pipe!\n");
			Sleep(1000);
			continue;
		}

		LOG_DEBUG("Connecting to output pipe!\n");
		result = ConnectNamedPipe(handleOutput, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
		LOG_DEBUG("Output pipe connected: %d\n", result);

		if(!result) {
			SAFE_CLOSE_HANDLE(handleOutput);
			logger.pipeHandle = NULL;
			continue;
		}

		// Logger pipe handle
		logger.pipeHandle = handleOutput;

		connectedOutput = true;

		// Read loop
		while(true) {
			if(logger.pipeHandle == NULL) {
				SAFE_CLOSE_HANDLE(handleOutput);
				break;
			}
			Sleep(100);
			/*
			result = ReadFile(handleOutput, &bufferRead, sizeof(bufferRead), &bytesRead, NULL);
			if(!result || bytesRead == 0) {
				if(GetLastError() == ERROR_BROKEN_PIPE) {
					LOG_DEBUG("Output pipe broken! %d\n", GetLastError());
					SAFE_CLOSE_HANDLE(handleOutput);
					break;
				}
			}
			*/
		}

	}

}


//
// Tablet state thread
//
void PipeHandler::RunStateThread()
{
	BOOL result;
	DWORD bytesWritten = 0;
	DWORD bytesRead = 0;
	char bufferWrite[1024];
	char bufferRead[1024];
	TabletState lastState;

	bool outputEnabled = false;

	// Connect loop
	while(true) {

		lock.lock();
		if(!isRunning) {
			lock.unlock();
			break;
		}
		else {
			lock.unlock();
		}

		connectedState = false;

		// Output connection
		handleState = CreateNamedPipeA(pipeNameState.c_str(),
			PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES,
			sizeof(bufferWrite),
			sizeof(bufferRead),
			500, // Timeout
			NULL
		);

		if(handleState == INVALID_HANDLE_VALUE) {
			LOG_ERROR("Couldn't create state pipe!\n");
			break;
		}

		LOG_DEBUG("Connecting to state pipe!\n");
		result = ConnectNamedPipe(handleState, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
		LOG_DEBUG("State pipe connected: %d\n", result);

		if(!result) {
			SAFE_CLOSE_HANDLE(handleState);
			logger.pipeHandle = NULL;
			Sleep(1000);
			continue;
		}

		connectedState = true;

		stateMessage.index = 0;

		// Write loop
		while(true) {

			lock.lock();
			outputEnabled = isStateOutputEnabled;
			lock.unlock();

			if(tablet != NULL && tabletHandler != NULL && outputEnabled) {
				if(tabletHandler->outputStateWrite.position.Distance(lastState.position) > 0) {

					// Input
					stateMessage.inputButtons = tablet->state.inputButtons;
					stateMessage.inputX = tablet->state.inputPosition.x;
					stateMessage.inputY = tablet->state.inputPosition.y;
					stateMessage.inputPressure = tablet->state.inputPressure;
					stateMessage.inputVelocity = tablet->state.inputVelocity;

					// Output
					stateMessage.outputButtons = tabletHandler->outputStateWrite.buttons;
					stateMessage.outputX = tabletHandler->outputStateWrite.position.x;
					stateMessage.outputY = tabletHandler->outputStateWrite.position.y;
					stateMessage.outputPressure = tabletHandler->outputStateWrite.pressure;

					// Index
					stateMessage.index++;

					// Write to pipe
					WriteFile(handleState, &stateMessage, sizeof(stateMessage), &bytesWritten, NULL);

					// Update last state
					memcpy(&lastState, &tabletHandler->outputStateWrite, sizeof(TabletState));
				}
				else {
					Sleep(2);
				}
			}
			else {
				Sleep(100);
			}

		}

	}

}