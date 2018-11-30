#pragma once
#include "CommandLine.h"
#include "TabletFilterTester.h"

bool ProcessCommand(CommandLine *cmd);
bool ReadCommandFile(string filename);
void LogTabletArea(string text);
void LogInformation();
void LogStatus();
bool CheckTablet();