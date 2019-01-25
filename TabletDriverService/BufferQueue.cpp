#include "precompiled.h"
#include "BufferQueue.h"


//
// Add item to write buffer
//
void BufferQueue::Push(void *buffer, int length)
{
	std::unique_lock<std::mutex> mlock(lockQueue);

	// Create write buffer item
	Item *item = new Item();
	item->buffer = new BYTE[length];
	item->length = length;

	// Copy data
	memcpy(item->buffer, buffer, length);

	// Add to buffer
	queue.push(item);

	// Notify queue change
	Notify();
}

//
// Remove first item from the queue
//
void BufferQueue::Pop()
{
	delete queue.front();
	queue.pop();
}


//
// Remove all items from the queue
//
void BufferQueue::Clear()
{
	std::unique_lock<std::mutex> mlock(lockQueue);
	while (!queue.empty()) {
		delete queue.front();
		queue.pop();
	}
}


//
// Wait for queue items
//
void BufferQueue::Wait()
{
	std::unique_lock<std::mutex> mlock(lockWait);

	// Check if the queue is empty
	bool isQueueEmpty = false;
	lockQueue.lock();
	isQueueEmpty = queue.empty();
	lockQueue.unlock();

	// Wait for items if queue is empty
	if (isQueueEmpty) {
		conditionBuffer.wait(mlock);
	}
}

//
// Notify buffer changes
//
void BufferQueue::Notify()
{
	conditionBuffer.notify_all();
}


//
// 
//
BufferQueue::Item* BufferQueue::Front()
{
	return queue.front();
}


//
// Queue empty?
//
int BufferQueue::IsEmpty()
{
	std::unique_lock<std::mutex> mlock(lockQueue);
	return queue.empty();
}


// Queue item constructor
BufferQueue::Item::Item()
{
	buffer = NULL;
	length = 0;
}

// Queue item destructor
BufferQueue::Item::~Item()
{
	if (buffer != NULL && length > 0) {
		delete buffer;
		buffer = NULL;
		length = 0;
	}
}
