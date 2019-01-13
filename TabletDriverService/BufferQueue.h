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

	queue<Item*> queue;
	mutex lockQueue;
	mutex lockWait;
	condition_variable conditionBuffer;
	void Push(void *buffer, int length);
	void Pop();
	void Clear();
	void Wait();
	void Notify();
	BufferQueue::Item* Front();
	int IsEmpty();
};
