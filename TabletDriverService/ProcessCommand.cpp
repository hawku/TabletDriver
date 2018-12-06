#include "stdafx.h"
#include "ProcessCommand.h"

#define LOG_MODULE ""
#include "Logger.h"


//
// Process Command
//
bool ProcessCommand(CommandLine *cmd) {

	bool logResult = false;

	LOG_INFO(">> %s\n", cmd->line.c_str());

	//
	// Execute command handler command
	//
	if(cmd->command.back() == '?') {
		logResult = true;
		cmd->command.pop_back();
	}
	if(commandHandler->IsValidCommand(cmd->command)) {
		bool result = commandHandler->ExecuteCommand(cmd->command, cmd);
		if(logResult) {
			LOG_INFO("Result: %s\n", result ? "True" : "False");
		}
		return result;

	}

	// Unknown
	else if(cmd->isValid) {
		LOG_WARNING("Unknown command: %s\n", cmd->line.c_str());
	}

	return true;
}

