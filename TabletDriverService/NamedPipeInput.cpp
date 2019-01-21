#include "precompiled.h"
#include "NamedPipeInput.h"

#define LOG_MODULE "PipeInput"
#include "Logger.h"

NamedPipeInput::NamedPipeInput(std::string pipeName) : NamedPipeServer(pipeName)
{

}


NamedPipeInput::~NamedPipeInput()
{
}


void NamedPipeInput::ProcessData(int clientId, BYTE *buffer, int length)
{
	if (!IsRunning()) return;
	if (_isStopping) return;

	// Copy buffer to char array
	char *cmdBuffer = new char[length + 1];
	memcpy(cmdBuffer, buffer, length);

	// Terminate char array
	cmdBuffer[length] = 0;

	// Create command string
	std::string commandText = "";
	commandText.append(cmdBuffer);
	delete cmdBuffer;

	// Process command
	CommandLine *cmd = new CommandLine(commandText);
	if (cmd->isValid) {
		ProcessCommand(cmd);
	}
	delete cmd;


}