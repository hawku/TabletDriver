#pragma once

#include "targetver.h"

#include <stdio.h>
#include <math.h>

#include "VMulti.h"
#include "Tablet.h"
#include "TabletHandler.h"
#include "ScreenMapper.h"
#include "OutputManager.h"
#include "CommandHandler.h"
#include "NamedPipeServer.h"
#include "NamedPipeInput.h"
#include "NamedPipeState.h"

// IntelliSense "fix"...
//#define memcpy memcpy

#define SAFE_CLOSE_HANDLE(handle) \
	if((handle) != NULL && (handle) != (INVALID_HANDLE_VALUE)) { \
		CloseHandle(handle); \
		handle = NULL; \
	};


// Global variables
extern VMulti *vmulti;
extern CommandHandler *commandHandler;
extern Tablet *tablet;
extern TabletHandler *tabletHandler;
extern OutputManager *outputManager;
extern ScreenMapper *mapper;
extern NamedPipeInput *pipeInput;
extern NamedPipeServer *pipeOutput;
extern NamedPipeState *pipeState;

// Cleanup and exit
extern void CleanupAndExit(int code);

// Process command
extern bool ProcessCommand(CommandLine *cmd);
