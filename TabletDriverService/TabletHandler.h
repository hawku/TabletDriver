#pragma once
#include "Tablet.h"
#include <thread>

class TabletHandler {
public:
	Tablet *tablet;
	thread *tabletInputThread;
	HANDLE timer;
	double timerInterval;
	bool isRunning;

	TabletHandler();
	~TabletHandler();

	bool Start();
	bool Stop();
	bool StartTimer();
	bool StopTimer();
	void ChangeTimerInterval(int newInterval);
	void RunTabletInputThread();

	void OnTimerTick();


private:
	static VOID CALLBACK TimerCallback(_In_ PVOID lpParameter, _In_ BOOLEAN TimerOrWaitFired) {
		TabletHandler *tabletHandler = (TabletHandler *)lpParameter;
		tabletHandler->OnTimerTick();
	}
};

