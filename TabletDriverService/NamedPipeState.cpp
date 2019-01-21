#include "precompiled.h"
#include "NamedPipeState.h"


NamedPipeState::NamedPipeState(std::string pipeName) : NamedPipeServer(pipeName)
{
	isStateOutputEnabled = false;
}

NamedPipeState::~NamedPipeState()
{
	Stop();
}

//
// Start
//
bool NamedPipeState::Start()
{
	if (!IsRunning()) {
		bool result = NamedPipeServer::Start();
		threadStateWriter = new std::thread(&NamedPipeState::RunStateWriterThread, this);
		return result;
	}
	return false;
}


//
// Stop
//
bool NamedPipeState::Stop()
{
	printf("Stopping state writer...\n");

	SetRunningState(false);

	Sleep(10);

	if (threadStateWriter != NULL) {
		try {
			threadStateWriter->join();
			printf("Stopping state writer stopped!\n");
		}
		catch (std::exception &e) {
			printf("State writer exception: %s\n", e.what());
		}
	}

	return true;
	//return NamedPipeServer::Stop();
}

//
// State writer thread
//
void NamedPipeState::RunStateWriterThread()
{
	TabletState lastState;
	bool outputEnabled = false;
	bool running = true;

	while (IsRunning()) {

		if (tablet != NULL && tabletHandler != NULL && isStateOutputEnabled) {
			if (tabletHandler->outputStateWrite.position.Distance(lastState.position) > 0) {

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
				Write(&stateMessage, sizeof(stateMessage));

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
