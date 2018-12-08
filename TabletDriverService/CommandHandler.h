#pragma once
#include "Command.h"
#include "TabletFilterTester.h"

#include <string>
#include <map>


class CommandHandler {
public:
	map<string, Command*> commands;
	map<string, string> aliases;
	map<string, string> aliasNames;
	map<string, string> help;

	CommandHandler();
	~CommandHandler();

	bool AddCommand(Command *command);
	bool AddAlias(string commandName, string alias);
	bool AddHelp(string commandName, string line);
	void CreateCommands();
	void CreateTabletCommands();
	void CreateFilterCommands();
	void CreateDeviceCommands();
	void CreateOtherCommands();

	bool IsValidCommand(string command);
	bool ExecuteCommand(string command);
	bool ExecuteCommand(CommandLine *cmd);
	bool ExecuteCommand(string command, string parameters);
	bool ExecuteCommand(string command, CommandLine *cmd);

	bool ExecuteFile(string filename);

};

