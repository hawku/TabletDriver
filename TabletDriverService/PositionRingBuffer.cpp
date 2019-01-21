#include "precompiled.h"
#include "PositionRingBuffer.h"


//
// Constructor
//
PositionRingBuffer::PositionRingBuffer() {
	maxLength = sizeof(buffer) / sizeof(Vector2D);
	length = 0;
	count = 0;
	index = 0;
	isValid = false;
}


//
// Destructor
//
PositionRingBuffer::~PositionRingBuffer() {
}


//
// Set buffer length
//
void PositionRingBuffer::SetLength(int len) {
	if(len > maxLength) {
		length = maxLength;
	}
	else {
		length = len;
	}
}


//
// Add position to buffer
//
void PositionRingBuffer::Add(Vector2D vector) {
	buffer[index].x = vector.x;
	buffer[index].y = vector.y;
	index++;
	count++;
	if(count > length) {
		count = length;
	}
	if(index >= length) {
		index = 0;
	}
	isValid = true;
}

//
// Move buffer positions towards a target with linear interpolation
//
void PositionRingBuffer::LerpAdd(Vector2D target, double t) {
	for(int i = 0; i < count; i++) {
		buffer[i].LerpAdd(target, t);
	}
}


//
// Get position history from the buffer
//
bool PositionRingBuffer::GetLatest(Vector2D *output, int delta) {
	int newIndex;

	// Buffer empty?
	if(count == 0) return false;

	// Valid delta?
	if(delta > 0 || delta <= -count) return false;

	newIndex = index - 1 + delta;

	// Limits
	if(newIndex < 0) newIndex = count + newIndex;

	if(newIndex < 0 || newIndex >= count) {
		return false;
	}

	output->x = buffer[newIndex].x;
	output->y = buffer[newIndex].y;
	return true;
}


//
// Reset buffer
//
void PositionRingBuffer::Reset() {
	count = 0;
	index = 0;
	isValid = false;
}



//
// [] operator
//
Vector2D *PositionRingBuffer::operator[](std::size_t index) {
	return &(buffer[index]);
}