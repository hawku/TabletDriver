#pragma once

#include "CommandLine.h"
#include <string>


class Command {
public:

	string name;
	function<bool(CommandLine*)> callback;

	bool Execute(CommandLine *cmd);

	Command();
	Command(string _name, function<bool(CommandLine*)> _callback);
	~Command();
};
