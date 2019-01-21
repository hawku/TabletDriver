#include "precompiled.h"
#include "Runnable.h"

bool Runnable::IsRunning()
{
	return _isRunning;
}

void Runnable::SetRunningState(bool running)
{
	_isRunning = running;
}
