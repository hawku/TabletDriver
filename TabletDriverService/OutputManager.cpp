#include "stdafx.h"
#include "OutputManager.h"

#define LOG_MODULE "OutputManager"
#include "Logger.h"


OutputManager::OutputManager() {
	output = &vmultiAbsolute;
	mode = ModeVMultiAbsolute;

	outputs[ModeVMultiAbsolute] = &vmultiAbsolute;
	outputs[ModeVMultiAbsoluteV2] = &vmultiAbsoluteV2;
	outputs[ModeVMultiRelative] = &vmultiRelative;
	outputs[ModeVMultiDigitizer] = &vmultiDigitizer;
	outputs[ModeSendInputAbsolute] = &sendInputAbsolute;
	outputs[ModeSendInputRelative] = &sendInputAbsolute;


	settings = new OutputSettings();

	for(int i = 0; i < (sizeof(outputs) / sizeof(Output*)); i++) {
		outputs[i]->settings = settings;
		LOG_DEBUG("LOOP!\n");
	}

}


OutputManager::~OutputManager() {
}

void OutputManager::SetOutputMode(OutputMode newMode) {

	// Reset old output
	if(newMode != mode) {
		Reset();
	}
	mode = newMode;

	switch(newMode) {
	case ModeVMultiAbsolute:
	case ModeVMultiAbsoluteV2:
	case ModeVMultiRelative:
	case ModeVMultiDigitizer:
		output = outputs[newMode];
		break;
	case ModeSendInputAbsolute:
		output = outputs[newMode];
		sendInputAbsolute.UpdateMonitorInfo();
		break;
	default:
		LOG_ERROR("Unknown Output Mode!\n");
		output = NULL;
		break;
	}

	//Reset();
}

bool OutputManager::Set(unsigned char buttons, double x, double y, double pressure) {
	if(output == NULL) return false;
	return output->Set(buttons, x, y, pressure);
}

bool OutputManager::Write() {
	if(output == NULL) return false;
	return output->Write();
}

bool OutputManager::Reset() {
	if(output == NULL) return false;
	return output->Reset();
}
