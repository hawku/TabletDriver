#pragma once

#include <atomic>

#include "NamedPipeServer.h"
class NamedPipeState : public NamedPipeServer
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

	std::thread *threadStateWriter;
	std::atomic<bool> isStateOutputEnabled;

	NamedPipeState(std::string pipeName);
	~NamedPipeState();

	bool Start() override;
	bool Stop() override;
	void RunStateWriterThread();

};

