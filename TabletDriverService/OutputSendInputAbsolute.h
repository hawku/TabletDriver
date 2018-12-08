#pragma once
class OutputSendInputAbsolute : public Output {
public:

	struct {
		double primaryWidth = GetSystemMetrics(SM_CXSCREEN);
		double primaryHeight = GetSystemMetrics(SM_CYSCREEN);
		double virtualWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		double virtualHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
		double virtualX = GetSystemMetrics(SM_XVIRTUALSCREEN);
		double virtualY = GetSystemMetrics(SM_YVIRTUALSCREEN);
	} monitorInfo;

	INPUT input = { 0 };
	INPUT lastInput = { 0 };
	unsigned char lastButtons;


	void UpdateMonitorInfo();

	void Init() override;
	bool Set(TabletState *tabletState);
	bool Write();
	bool Reset();


	OutputSendInputAbsolute();
	~OutputSendInputAbsolute();
};

