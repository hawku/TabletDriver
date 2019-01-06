#include "stdafx.h"
#include "Runnable.h"

bool Runnable::IsRunning()
{
	bool running = false;
	isRunningLock.lock();
	running = _isRunning;
	isRunningLock.unlock();
	return running;
}

void Runnable::SetRunningState(bool running)
{
	isRunningLock.lock();
	_isRunning = running;
	isRunningLock.unlock();
}
