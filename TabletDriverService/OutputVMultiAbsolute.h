#pragma once
#include "Output.h"

class OutputVMultiAbsolute : public Output {
public:

	// Absolute mouse vmulti report
	struct {
		BYTE vmultiId;
		BYTE reportLength;
		BYTE reportId;
		unsigned char buttons;
		USHORT x;
		USHORT y;
		BYTE wheel;
	} report;

	bool Set(unsigned char buttons, double x, double y, double pressure);
	bool Write();
	bool Reset();

	OutputVMultiAbsolute();
	~OutputVMultiAbsolute();
};

