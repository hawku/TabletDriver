#pragma once
#include "Command.h"
#include "TabletFilterTester.h"

#include <string>
#include <map>


class CommandHandler {
public:
	map<string, Command*> commands;	
	map<string, string> aliases;

	CommandHandler();
	~CommandHandler();

	bool AddCommand(Command *command);
	bool AddAlias(string commandName, string alias);
	void CreateCommands();

	bool IsValidCommand(string command);
	bool ExecuteCommand(string command);
	bool ExecuteCommand(CommandLine *cmd);
	bool ExecuteCommand(string command, string parameters);
	bool ExecuteCommand(string command, CommandLine *cmd);

	bool ExecuteFile(string filename);

};

