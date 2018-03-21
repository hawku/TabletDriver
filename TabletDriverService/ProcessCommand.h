#pragma once
#include "CommandLine.h"
#include "Utils.h"

bool ProcessCommand(CommandLine *cmd);
bool ReadCommandFile(string filename);
void LogTabletArea(string text);
void LogInformation();
bool CheckTablet();