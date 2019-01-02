#pragma once
class OutputSendInputRelative : public Output {
public:

	INPUT input = { 0 };
	INPUT lastInput = { 0 };
	unsigned char lastButtons;

	void Init() override;
	bool Set(TabletState *tabletState);
	bool Write();
	bool Reset();

	OutputSendInputRelative();
	~OutputSendInputRelative();
};

