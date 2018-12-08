#include "stdafx.h"
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
}



//
// Add command
//
bool CommandHandler::AddCommand(Command *command) {
	string name = command->name;
	transform(name.begin(), name.end(), name.begin(), ::tolower);
	if(commands.count(name) <= 0) {
		commands.insert(pair<string, Command*>(name, command));
		return true;
	}
	return false;
}

//
// Add command alias
//
bool CommandHandler::AddAlias(string alias, string commandName) {

	string aliasLowerCase = alias;
	transform(aliasLowerCase.begin(), aliasLowerCase.end(), aliasLowerCase.begin(), ::tolower);
	transform(commandName.begin(), commandName.end(), commandName.begin(), ::tolower);

	if(aliases.count(aliasLowerCase) <= 0) {
		aliases.insert(pair<string, string>(aliasLowerCase, commandName));
		aliasNames.insert(pair<string, string>(aliasLowerCase, alias));
		return true;
	}

	return false;
}

//
// Add command help text
//
bool CommandHandler::AddHelp(string commandName, string line) {
	transform(commandName.begin(), commandName.end(), commandName.begin(), ::tolower);
	line.append("\n");
	if(help.count(commandName) < 0) {
		help.insert(pair<string, string>(commandName, line));
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
	CreateFilterCommands();
	CreateOtherCommands();

}

//
// Is valid command?
//
bool CommandHandler::IsValidCommand(string command) {
	transform(command.begin(), command.end(), command.begin(), ::tolower);
	if(aliases.count(command) > 0) {
		command = aliases[command];
	}
	if(commands.count(command) > 0) {
		return true;
	}
	return false;
}

//
// Execute a command using command name
//
bool CommandHandler::ExecuteCommand(string command) {
	return ExecuteCommand(command, NULL);
}

//
// Execute a command using command line
//
bool CommandHandler::ExecuteCommand(CommandLine *cmd) {
	return ExecuteCommand(cmd->command, cmd);
}

//
// Execute a command using command and parameter string
//
bool CommandHandler::ExecuteCommand(string command, string parameters) {
	CommandLine *cmd = new CommandLine(command + " " + parameters);
	bool result = ExecuteCommand(command, cmd);
	delete cmd;
	return result;
}

//
// Execute a command
//
bool CommandHandler::ExecuteCommand(string command, CommandLine * cmd) {
	transform(command.begin(), command.end(), command.begin(), ::tolower);
	if(aliases.count(command) > 0) {
		command = aliases[command];
	}
	if(commands.count(command) > 0) {
		return commands[command]->Execute(cmd);
	}
	return false;
}

//
// Execute commands from a file
//
bool CommandHandler::ExecuteFile(string filename) {

	CommandLine *cmd;
	ifstream file;
	string line = "";

	// Open file
	file.open(filename);
	if(!file.is_open()) {
		return false;
	}


	LOG_INFO("\\ Reading '%s'\n", filename.c_str());

	// Loop through lines
	while(!file.eof()) {
		getline(file, line);
		if(line.length() == 0) continue;
		cmd = new CommandLine(line);

		//
		// Do not redefine tablet if one is already open
		//
		if(
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

		ExecuteCommand(cmd);

		delete cmd;
	}
	file.close();

	LOG_INFO("/ End of '%s'\n", filename.c_str());

	return true;
}
