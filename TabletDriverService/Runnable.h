#pragma once

#include <mutex>

class Runnable
{
protected:
	bool _isRunning = false;
	mutex isRunningLock;
public:
	bool IsRunning();
	void SetRunningState(bool running);
};

