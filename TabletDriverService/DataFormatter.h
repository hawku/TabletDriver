#pragma once
class DataFormatter
{
public:
	class DataInstruction {
	public:
		enum WriteMode {
			WriteModeOR,
			WriteModeAND,
			WriteModeSet
		};

		int targetByte;
		int targetBitMask;

		int sourceByte;
		int sourceBitShift;
		int sourceBitMask;

		int writeMode;
		
		DataInstruction();
	};

	DataInstruction instructions[128];
	int instructionCount;
	int targetLength;
	int sourceLength;

	DataFormatter();
	~DataFormatter();

	bool Format(void *targetBuffer, void *sourceBuffer);
	bool AddInstruction(int targetByte, int targetBitMask, int sourceByte, int sourceBitMask, int sourceBitShift, int writeMode);
	bool AddInstruction(DataInstruction *instruction);


};

