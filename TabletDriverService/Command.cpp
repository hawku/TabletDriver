#include "stdafx.h"
#include "Command.h"


bool Command::Execute(CommandLine *cmd) {
	if(callback != NULL) {
		return callback(cmd);
	}
	return false;
}

Command::Command() {
	name = "";
	callback = NULL;
}

Command::Command(string _name, function<bool(CommandLine*)> _callback) {
	this->name = _name;
	this->callback = _callback;
}

Command::~Command() {
}
