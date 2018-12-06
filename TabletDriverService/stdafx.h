// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#pragma once
#include "targetver.h"

#include <windows.h>
#include <stdio.h>
#include <math.h>

#include "VMulti.h"
#include "Tablet.h"
#include "ScreenMapper.h"
#include "OutputManager.h"
#include "TabletHandler.h"
#include "CommandHandler.h"

// Global variables...
extern VMulti *vmulti;
extern CommandHandler *commandHandler;
extern Tablet *tablet;
extern TabletHandler *tabletHandler;
extern OutputManager *outputManager;
extern ScreenMapper *mapper;
extern void CleanupAndExit(int code);


// TODO: reference additional headers your program requires here
