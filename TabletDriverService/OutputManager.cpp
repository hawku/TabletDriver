#include "stdafx.h"
#include "OutputManager.h"

#define LOG_MODULE "OutputManager"
#include "Logger.h"


OutputManager::OutputManager() {
	output = &vmultiAbsolute;
	mode = ModeVMultiAbsolute;

	// Set outputs array
	outputs[ModeVMultiAbsolute] = &vmultiAbsolute;
	outputs[ModeVMultiAbsoluteV2] = &vmultiAbsoluteV2;
	outputs[ModeVMultiRelative] = &vmultiRelative;
	outputs[ModeVMultiDigitizer] = &vmultiDigitizer;
	outputs[ModeSendInputAbsolute] = &sendInputAbsolute;
	outputs[ModeSendInputRelative] = &sendInputAbsolute;
	outputs[ModeDummy] = &dummy;

	// Use same settings on all outputs
	settings = new OutputSettings();
	for(int i = 0; i < (sizeof(outputs) / sizeof(Output*)); i++) {
		outputs[i]->settings = settings;
	}

}


OutputManager::~OutputManager() {
	if(settings != NULL)
		delete settings;
}

void OutputManager::SetOutputMode(OutputMode newMode) {

	// Reset old output
	if(newMode != mode) {
		Reset();
	}
	mode = newMode;

	// Set new output
	output = outputs[newMode];

	// Initialize new output
	output->Init();

}

bool OutputManager::Set(TabletState *tabletState) {
	if(output == NULL) return false;
	return output->Set(tabletState);
}

bool OutputManager::Write() {
	if(output == NULL) return false;
	return output->Write();
}

bool OutputManager::Reset() {
	if(output == NULL) return false;
	return output->Reset();
}
