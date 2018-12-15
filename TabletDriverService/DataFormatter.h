#pragma once
class DataFormatter
{
public:
	class DataInstruction {
	public:
		int targetByte;
		int targetBitMask;

		int sourceByte;
		int sourceBitShift;
		int sourceBitMask;

		DataInstruction();
	};

	DataInstruction instructions[128];
	int instructionCount;
	int targetLength;
	int sourceLength;

	DataFormatter();
	~DataFormatter();

	bool Format(void *targetBuffer, void *sourceBuffer);
	bool AddInstruction(int targetByte, int targetBitMask, int sourceByte, int sourceBitMask, int sourceBitShift);
	bool AddInstruction(DataInstruction *instruction);


};

