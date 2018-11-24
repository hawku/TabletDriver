#pragma once

#include "Output.h"
#include "OutputVMultiAbsolute.h"
#include "OutputVMultiAbsoluteV2.h"
#include "OutputVMultiRelative.h"
#include "OutputVMultiDigitizer.h"
#include "OutputSendInputAbsolute.h"


class OutputManager : public Output {
public:

	enum OutputMode {
		ModeVMultiAbsolute,
		ModeVMultiAbsoluteV2,
		ModeVMultiRelative,
		ModeVMultiDigitizer,
		ModeSendInputAbsolute,
		ModeSendInputRelative,
	};

	Output * output;
	Output *outputs[6];
	
	OutputVMultiAbsolute vmultiAbsolute;
	OutputVMultiAbsoluteV2 vmultiAbsoluteV2;
	OutputVMultiRelative vmultiRelative;
	OutputVMultiDigitizer vmultiDigitizer;
	OutputSendInputAbsolute sendInputAbsolute;
	
	OutputMode mode;

	void SetOutputMode(OutputMode newMode);

	bool Set(unsigned char buttons, double x, double y, double pressure);
	bool Write();
	bool Reset();

	OutputManager();
	~OutputManager();

};

