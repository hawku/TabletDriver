#pragma once
#include "NamedPipeServer.h"
class NamedPipeInput : public NamedPipeServer
{
public:
	NamedPipeInput(string pipeName);
	~NamedPipeInput();

	int ProcessData(int clientId, char *bufferInput, int length, char *bufferOutput) override;

};

