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

	double maxPressure;

	bool Set(TabletState *tabletState);
	bool Write();
	bool Reset();

	OutputVMultiDigitizer();
	~OutputVMultiDigitizer();
};

