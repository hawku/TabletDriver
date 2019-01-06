#include "stdafx.h"
#include "NamedPipeInput.h"

#define LOG_MODULE "PipeInput"
#include "Logger.h"

NamedPipeInput::NamedPipeInput(string pipeName) : NamedPipeServer(pipeName)
{

}


NamedPipeInput::~NamedPipeInput()
{
}

int NamedPipeInput::ProcessData(int clientId, char *buffer, int length, char * bufferOutput)
{
	buffer[length] = 0;
	//LOG_DEBUG("PipeInput data: %s\n", buffer);
	CommandLine *cmd = new CommandLine(buffer);
	ProcessCommand(cmd);
	delete cmd;

	return 0;
}
