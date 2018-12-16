#pragma once

#include "Output.h"
#include "OutputVMultiAbsolute.h"
#include "OutputVMultiRelative.h"
#include "OutputVMultiDigitizer.h"
#include "OutputSendInputAbsolute.h"
#include "OutputDummy.h"

class OutputManager : public Output {
public:

	enum OutputMode {
		ModeVMultiAbsolute,
		ModeVMultiRelative,
		ModeVMultiDigitizer,
		ModeSendInputAbsolute,
		ModeSendInputRelative,
		ModeDummy
	};

	Output *output;
	Output *outputs[6];

	OutputVMultiAbsolute vmultiAbsolute;
	OutputVMultiRelative vmultiRelative;
	OutputVMultiDigitizer vmultiDigitizer;
	OutputSendInputAbsolute sendInputAbsolute;
	OutputDummy dummy;

	OutputMode mode;

	void SetOutputMode(OutputMode newMode);

	bool Set(TabletState *tabletState);
	bool Write();
	bool Reset();

	OutputManager();
	~OutputManager();

};

