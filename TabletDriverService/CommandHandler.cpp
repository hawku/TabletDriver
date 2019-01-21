#include "precompiled.h"
#include "CommandHandler.h"

#define LOG_MODULE ""
#include "Logger.h"


//
// Constructor
//
CommandHandler::CommandHandler() {
}


//
// Destructor
//
CommandHandler::~CommandHandler() {

	// Wait locked commands to finish
	lock.lock();
	lock.unlock();
}



//
// Add command
//
bool CommandHandler::AddCommand(Command *command) {
	std::string name = command->name;
	std::transform(name.begin(), name.end(), name.begin(), ::tolower);
	if (commands.count(name) <= 0) {
		commands.insert(std::pair<std::string, Command*>(name, command));
		return true;
	}
	return false;
}

//
// Add command alias
//
bool CommandHandler::AddAlias(std::string alias, std::string commandName) {

	std::string aliasLowerCase = alias;
	std::transform(aliasLowerCase.begin(), aliasLowerCase.end(), aliasLowerCase.begin(), ::tolower);
	std::transform(commandName.begin(), commandName.end(), commandName.begin(), ::tolower);

	if (aliases.count(aliasLowerCase) <= 0) {
		aliases.insert(std::pair<std::string, std::string>(aliasLowerCase, commandName));
		aliasNames.insert(std::pair<std::string, std::string>(aliasLowerCase, alias));
		return true;
	}

	return false;
}

//
// Add command help text
//
bool CommandHandler::AddHelp(std::string commandName, std::string line) {
	std::transform(commandName.begin(), commandName.end(), commandName.begin(), ::tolower);
	line.append("\n");
	if (help.count(commandName) < 0) {
		help.insert(std::pair<std::string, std::string>(commandName, line));
	}
	else {
		help[commandName].append(line);
	}
	return false;
}

//
// Create commands
//
void CommandHandler::CreateCommands() {

	CreateDeviceCommands();
	CreateTabletCommands();
	CreateAuxCommands();
	CreateFilterCommands();
	CreateOtherCommands();

}

//
// Is valid command?
//
bool CommandHandler::IsValidCommand(std::string command) {
	std::transform(command.begin(), command.end(), command.begin(), ::tolower);
	if (aliases.count(command) > 0) {
		command = aliases[command];
	}
	if (commands.count(command) > 0) {
		return true;
	}
	return false;
}

//
// Execute a command using command name
//
bool CommandHandler::ExecuteCommand(std::string command) {
	return ExecuteCommand(command, NULL);
}
bool CommandHandler::ExecuteCommandLock(std::string command) {
	std::unique_lock<std::mutex> mlock(lock);
	return ExecuteCommand(command);
}


//
// Execute a command using command line
//
bool CommandHandler::ExecuteCommand(CommandLine *cmd) {
	return ExecuteCommand(cmd->command, cmd);
}
bool CommandHandler::ExecuteCommandLock(CommandLine *cmd) {
	std::unique_lock<std::mutex> mlock(lock);
	return ExecuteCommand(cmd);
}


//
// Execute a command using command and parameter string
//
bool CommandHandler::ExecuteCommand(std::string command, std::string parameters) {
	CommandLine *cmd = new CommandLine(command + " " + parameters);
	bool result = ExecuteCommand(command, cmd);
	delete cmd;
	return result;
}
bool CommandHandler::ExecuteCommandLock(std::string command, std::string parameters) {
	std::unique_lock<std::mutex> mlock(lock);
	return ExecuteCommand(command, parameters);
}


//
// Execute a command
//
bool CommandHandler::ExecuteCommand(std::string command, CommandLine * cmd) {
	std::transform(command.begin(), command.end(), command.begin(), ::tolower);
	if (aliases.count(command) > 0) {
		command = aliases[command];
	}
	if (commands.count(command) > 0) {
		return commands[command]->Execute(cmd);
	}
	return false;
}
bool CommandHandler::ExecuteCommandLock(std::string command, CommandLine * cmd) {
	std::unique_lock<std::mutex> mlock(lock);
	return ExecuteCommand(command, cmd);
}


//
// Execute commands from a file
//
bool CommandHandler::ExecuteFile(std::string filename) {

	CommandLine *cmd;
	std::ifstream file;
	std::string line = "";

	// Open file
	file.open(filename);
	if (!file.is_open()) {
		return false;
	}

	LOG_INFO("\\ Reading '%s'\n", filename.c_str());

	// Loop through lines
	while (!file.eof()) {
		getline(file, line);
		if (line.length() == 0) continue;
		cmd = new CommandLine(line);

		//
		// Do not redefine tablet if one is already open
		//
		if (
			(
				cmd->is("Tablet")
				||
				cmd->is("USBTablet")
				||
				cmd->is("HIDTablet")
				)
			&&
			tablet != NULL &&
			tablet->IsConfigured()
			) {
			LOG_INFO(">> %s\n", cmd->line.c_str());
			LOG_INFO("Tablet is already defined!\n");
			delete cmd;
			break;
		}
		LOG_INFO(">> %s\n", cmd->line.c_str());

		std::string command = cmd->command;
		std::transform(command.begin(), command.end(), command.begin(), ::tolower);
		if (aliases.count(command) > 0) {
			command = aliases[command];
		}
		if (commands.count(command) > 0) {
			commands[command]->Execute(cmd);
		}
		//ExecuteCommand(cmd);

		delete cmd;
	}
	file.close();

	LOG_INFO("/ End of '%s'\n", filename.c_str());

	return true;
}

bool CommandHandler::ExecuteFileLock(std::string filename)
{
	std::unique_lock<std::mutex> mlock(lock);
	return ExecuteFile(filename);
}
