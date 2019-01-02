#pragma once
#include "Output.h"
class OutputVMultiDigitizerRelative : public Output
{
public:

	// Digitizer vmulti report
	struct {
		BYTE vmultiId;
		BYTE reportLength;
		BYTE reportId;
		BYTE buttons;
		USHORT x;
		USHORT y;
		USHORT pressure;
	} report;

	struct {
		double primaryWidth = GetSystemMetrics(SM_CXSCREEN);
		double primaryHeight = GetSystemMetrics(SM_CYSCREEN);
		double virtualWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		double virtualHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
		double virtualX = GetSystemMetrics(SM_XVIRTUALSCREEN);
		double virtualY = GetSystemMetrics(SM_YVIRTUALSCREEN);
	} monitorInfo;

	Vector2D absolutePosition;

	double maxPressure;

	OutputVMultiDigitizerRelative();
	~OutputVMultiDigitizerRelative();

	void UpdateMonitorInfo();
	void Init() override;
	bool Set(TabletState *tabletState);
	bool Write();
	bool Reset();
};

