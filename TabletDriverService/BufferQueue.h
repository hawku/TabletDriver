#pragma once

#include <queue>

class BufferQueue {
public:

	class Item {
	public:
		BYTE *buffer;
		int length;
		Item();
		~Item();
	};

	std::queue<Item*> queue;
	std::mutex lockQueue;
	std::mutex lockWait;
	std::condition_variable conditionBuffer;
	void Push(void *buffer, int length);
	void Pop();
	void Clear();
	void Wait();
	void Notify();
	BufferQueue::Item* Front();
	int IsEmpty();
};
