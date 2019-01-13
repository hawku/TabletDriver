#pragma once

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <string>

#include "Runnable.h"
#include "BufferQueue.h"

constexpr auto BUFFER_SIZE = 4096;
constexpr auto MAX_CLIENTS = 16;

class NamedPipeServer : public Runnable
{
protected:
	atomic<bool> _isStopping;
public:
	string pipePath;
	string pipeName;
	thread *threadMain;
	mutex lockServer;

	class Client {
	private:
		atomic<bool> _isConnected;
	public:
		int id;
		HANDLE handlePipe;
		OVERLAPPED overlapped;
		NamedPipeServer *server;
		thread* threadRead;
		thread* threadWrite;
		string pipeName;
		mutex lockClient;
		BufferQueue writeQueue;
		mutex lockWriteWait;
		condition_variable conditionWriteQueue;
		Client();
		~Client();
		bool IsConnected();
		void SetConnectedState(bool connected);

		void RunReadThread();
		void RunWriteThread();
	};

	Client clients[MAX_CLIENTS];
	int clientCount;

	NamedPipeServer(string pipeName);
	~NamedPipeServer();

	virtual bool Start();
	virtual bool Stop();
	virtual void ProcessData(int clientId, BYTE *bufferInput, int length);
	bool Write(void * buffer, int length);
	bool IsClientConnected();

	void RunMainThread();



};

