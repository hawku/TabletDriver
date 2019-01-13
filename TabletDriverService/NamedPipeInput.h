#pragma once
#include "NamedPipeServer.h"
class NamedPipeInput : public NamedPipeServer
{
public:
	NamedPipeInput(string pipeName);
	~NamedPipeInput();

	void ProcessData(int clientId, BYTE *bufferInput, int length) override;

};

