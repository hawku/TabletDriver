#pragma once

#include <thread>
#include <mutex>

class PipeHandler
{
public:

#pragma pack(1)
	struct {
		int index;

		int inputButtons;
		double inputX;
		double inputY;
		double inputPressure;
		double inputVelocity;

		int outputButtons;
		double outputX;
		double outputY;
		double outputPressure;
	} stateMessage;

	string pipeNameInput;
	string pipeNameOutput;
	string pipeNameState;

	thread *threadInput;
	thread *threadOutput;
	thread *threadState;

	HANDLE handleInput;
	HANDLE handleOutput;
	HANDLE handleState;

	bool connectedInput;
	bool connectedOutput;
	bool connectedState;

	bool isRunning;
	bool isStateOutputEnabled;

	PipeHandler(string pipeName);
	~PipeHandler();

	bool Start();
	bool Stop();
	void RunInputThread();
	void RunOutputThread();
	void RunStateThread();

};

