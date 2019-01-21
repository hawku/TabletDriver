#include "precompiled.h"
#include "DataFormatter.h"

//#define LOG_MODULE "DataFormatter"
//#include "Logger.h"


DataFormatter::DataFormatter()
{
	instructionCount = 0;
	targetLength = 0;
	sourceLength = 0;
}


DataFormatter::~DataFormatter()
{
}

bool DataFormatter::Format(void * targetBuffer, void * sourceBuffer)
{
	UCHAR *target = (UCHAR*)targetBuffer;
	UCHAR *source = (UCHAR*)sourceBuffer;
	DataInstruction *instruction;
	unsigned int byte;

	// Loop through instructions
	for (int i = 0; i < instructionCount; i++) {
		instruction = &instructions[i];

		// Get source byte
		byte = source[instruction->sourceByte];

		// Source bit mask
		byte &= instruction->sourceBitMask;

		// Shift
		if (instruction->sourceBitShift < 0) {
			byte >>= -instruction->sourceBitShift;
		}
		else {
			byte <<= instruction->sourceBitShift;
		}

		// Target bit mask
		byte &= instruction->targetBitMask;

		// Write target byte
		switch (instruction->writeMode) {
		case DataInstruction::WriteModeOR: target[instruction->targetByte] |= byte & 0xFF;  break;
		case DataInstruction::WriteModeAND: target[instruction->targetByte] &= byte & 0xFF;  break;
		case DataInstruction::WriteModeSet: target[instruction->targetByte] = byte & 0xFF;  break;
		default: break;
		}

		
		/*
		LOG_DEBUG("  INSTR T=%d TM=0x%02X S=%d SM=0x%02X SS=%d R=%d\n",
			instruction->targetByte,
			instruction->targetBitMask,
			instruction->sourceByte,
			instruction->sourceBitMask,
			instruction->sourceBitShift,
			byte
		);
		*/
		
	}

	return true;
}

bool DataFormatter::AddInstruction(int targetByte, int targetBitMask, int sourceByte, int sourceBitMask, int sourceBitShift, int writeMode)
{
	if (instructionCount >= 128) return false;

	// Lengths
	if (targetByte >= targetLength) targetLength = targetByte + 1;
	if (sourceByte >= sourceLength) sourceLength = sourceByte + 1;

	instructions[instructionCount].targetByte = targetByte;
	instructions[instructionCount].targetBitMask = targetBitMask;
	instructions[instructionCount].sourceByte = sourceByte;
	instructions[instructionCount].sourceBitMask = sourceBitMask;
	instructions[instructionCount].sourceBitShift = sourceBitShift;
	instructions[instructionCount].writeMode = writeMode;
	instructionCount++;
	return true;
}

bool DataFormatter::AddInstruction(DataInstruction * instruction)
{
	return AddInstruction(
		instruction->targetByte,
		instruction->targetBitMask,
		instruction->sourceByte,
		instruction->sourceBitMask,
		instruction->sourceBitShift,
		instruction->writeMode
	);
}

DataFormatter::DataInstruction::DataInstruction()
{
	targetByte = 0;
	targetBitMask = 0xFF;
	sourceByte = 0;
	sourceBitMask = 0xFF;
	sourceBitShift = 0;
}
