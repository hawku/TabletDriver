#pragma once
#include "Command.h"
#include "TabletFilterTester.h"

#include <mutex>
#include <string>
#include <map>


class CommandHandler {
public:
	std::map<std::string, Command*> commands;
	std::map<std::string, std::string> aliases;
	std::map<std::string, std::string> aliasNames;
	std::map<std::string, std::string> help;

	std::mutex lock;

	CommandHandler();
	~CommandHandler();

	bool AddCommand(Command *command);
	bool AddAlias(std::string commandName, std::string alias);
	bool AddHelp(std::string commandName, std::string line);
	void CreateCommands();
	void CreateTabletCommands();
	void CreateFilterCommands();
	void CreateDeviceCommands();
	void CreateAuxCommands();
	void CreateOtherCommands();

	bool IsValidCommand(std::string command);
	bool ExecuteCommand(std::string command);
	bool ExecuteCommandLock(std::string command);
	bool ExecuteCommand(CommandLine *cmd);
	bool ExecuteCommandLock(CommandLine * cmd);
	bool ExecuteCommand(std::string command, std::string parameters);
	bool ExecuteCommandLock(std::string command, std::string parameters);
	bool ExecuteCommand(std::string command, CommandLine *cmd);
	bool ExecuteCommandLock(std::string command, CommandLine * cmd);

	bool ExecuteFile(std::string filename);
	bool ExecuteFileLock(std::string filename);

};

