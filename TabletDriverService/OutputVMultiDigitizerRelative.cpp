#include "precompiled.h"
#include "OutputVMultiDigitizerRelative.h"

#define LOG_MODULE "DigitizerRelative"
#include "Logger.h"


//
// Constructor
//
OutputVMultiDigitizerRelative::OutputVMultiDigitizerRelative()
{

	// Digitizer VMulti report
	report.reportLength = 8;
	report.buttons = 0;
	report.x = 0;
	report.y = 0;
	report.pressure = 0;

	// XP-Pen
	if(vmulti->type == VMulti::TypeXPPen) {
		report.vmultiId = 0x40;
		report.reportId = 5;
		maxPressure = 2047.0;
	}

	// VEIKK
	else if(vmulti->type == VMulti::TypeVEIKK) {
		report.vmultiId = 0x09;
		report.reportId = 2;
		maxPressure = 8191.0;
	}

	absolutePosition.Set(0, 0);
	UpdateMonitorInfo();

	absolutePosition.x = monitorInfo.virtualWidth / 2.0;
	absolutePosition.y = monitorInfo.virtualHeight / 2.0;

}

//
// Destructor
//
OutputVMultiDigitizerRelative::~OutputVMultiDigitizerRelative()
{
}

//
// Update monitor info
//
void OutputVMultiDigitizerRelative::UpdateMonitorInfo()
{
	monitorInfo.primaryWidth = GetSystemMetrics(SM_CXSCREEN);
	monitorInfo.primaryHeight = GetSystemMetrics(SM_CYSCREEN);
	monitorInfo.virtualWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	monitorInfo.virtualHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	monitorInfo.virtualX = GetSystemMetrics(SM_XVIRTUALSCREEN);
	monitorInfo.virtualY = GetSystemMetrics(SM_YVIRTUALSCREEN);
}


//
// Initialize output
//
void OutputVMultiDigitizerRelative::Init()
{
	UpdateMonitorInfo();
}


//
// Set output
//
bool OutputVMultiDigitizerRelative::Set(TabletState *tabletState)
{
	// Get relative position delta
	Vector2D delta;
	outputManager->GetRelativePositionDelta(tabletState, &delta);

	// Move absolute position
	absolutePosition.Add(delta);

	if(delta.x == 0 && delta.y == 0) return true;

	// Limits
	if(absolutePosition.x < 0) absolutePosition.x = 0;
	else if(absolutePosition.x > monitorInfo.virtualWidth) absolutePosition.x = monitorInfo.virtualWidth;
	if(absolutePosition.y < 0) absolutePosition.y = 0;
	else if(absolutePosition.y > monitorInfo.virtualHeight) absolutePosition.y = monitorInfo.virtualHeight;


	report.buttons = tabletState->buttons | 0x20;
	report.x = (USHORT)round(absolutePosition.x / monitorInfo.virtualWidth * 32767.0);
	report.y = (USHORT)round(absolutePosition.y / monitorInfo.virtualHeight * 32767.0);
	report.pressure = (USHORT)round(tabletState->pressure * maxPressure);
	vmulti->SetReport(&report, sizeof(report));

	return true;
}


//
// Write output
//
bool OutputVMultiDigitizerRelative::Write()
{
	// Write report to VMulti device if report has changed
	if(vmulti->HasReportChanged()) {

		// Debug message
		if(logger.IsDebugOutputEnabled()) {
			LOG_DEBUGBUFFER(&report, 10, "Report: ");
		}

		vmulti->WriteReport();
		return true;
	}
	return false;
}


//
// Reset output
//
bool OutputVMultiDigitizerRelative::Reset()
{
	// Do not reset when buttons are not pressed
	if(report.buttons == 0) {
		return true;
	}

	report.buttons = 0;
	report.pressure = 0;
	vmulti->SetReport(&report, sizeof(report));
	vmulti->WriteReport();
	return true;
}

