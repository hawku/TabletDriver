#pragma once
#include "Output.h"

class OutputVMultiAbsoluteV2 : public Output {
public:

	// Absolute mouse vmulti report
	struct {
		BYTE vmultiId;
		BYTE reportLength;
		BYTE reportId;
		unsigned char buttons;
		USHORT x;
		USHORT y;
		USHORT pressure;
	} report;

	bool Set(unsigned char buttons, double x, double y, double pressure);
	bool Write();
	bool Reset();

	OutputVMultiAbsoluteV2();
	~OutputVMultiAbsoluteV2();
};

