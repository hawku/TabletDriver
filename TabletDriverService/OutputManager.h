#pragma once

#include "Output.h"
#include "OutputVMultiAbsolute.h"
#include "OutputVMultiAbsoluteV2.h"
#include "OutputVMultiRelative.h"
#include "OutputVMultiDigitizer.h"
#include "OutputSendInputAbsolute.h"
#include "OutputDummy.h"

class OutputManager : public Output {
public:

	enum OutputMode {
		ModeVMultiAbsolute,
		ModeVMultiAbsoluteV2,
		ModeVMultiRelative,
		ModeVMultiDigitizer,
		ModeSendInputAbsolute,
		ModeSendInputRelative,
		ModeDummy
	};

	Output *output;
	Output *outputs[7];

	OutputVMultiAbsolute vmultiAbsolute;
	OutputVMultiAbsoluteV2 vmultiAbsoluteV2;
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

