#pragma once

#include <atomic>
#include <mutex>

class Runnable
{
protected:
	atomic<bool> _isRunning = false;
public:
	bool IsRunning();
	void SetRunningState(bool running);
};

