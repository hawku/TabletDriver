#pragma once

#include <atomic>
#include <mutex>

class Runnable
{
protected:
	std::atomic<bool> _isRunning = false;
public:
	bool IsRunning();
	void SetRunningState(bool running);
};

