#pragma once

#include "Output.h"
#include "OutputVMultiAbsolute.h"
#include "OutputVMultiRelative.h"
#include "OutputVMultiDigitizer.h"
#include "OutputVMultiDigitizerRelative.h"
#include "OutputSendInputAbsolute.h"
#include "OutputSendInputRelative.h"
#include "OutputDummy.h"

class OutputManager : public Output {
public:

	enum OutputMode {
		ModeVMultiAbsolute,
		ModeVMultiRelative,
		ModeVMultiDigitizer,
		ModeVMultiDigitizerRelative,
		ModeSendInputAbsolute,
		ModeSendInputRelative,
		ModeDummy
	};

	Output *output;
	Output *outputs[7];

	OutputVMultiAbsolute vmultiAbsolute;
	OutputVMultiRelative vmultiRelative;
	OutputVMultiDigitizer vmultiDigitizer;
	OutputVMultiDigitizerRelative vmultiDigitizerRelative;
	OutputSendInputAbsolute sendInputAbsolute;
	OutputSendInputRelative sendInputRelative;
	OutputDummy dummy;

	OutputMode mode;

	void SetOutputMode(OutputMode newMode);

	bool Set(TabletState *tabletState);
	bool Write();
	bool Reset();

	bool GetRelativePositionDelta(TabletState *tabletState, Vector2D *delta);


	OutputManager();
	~OutputManager();

};

