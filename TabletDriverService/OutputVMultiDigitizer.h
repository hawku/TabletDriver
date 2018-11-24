#pragma once
class OutputVMultiDigitizer : public Output {
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

	bool Set(unsigned char buttons, double x, double y, double pressure);
	bool Write();
	bool Reset();

	OutputVMultiDigitizer();
	~OutputVMultiDigitizer();
};

