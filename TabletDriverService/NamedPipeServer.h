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
	std::atomic<bool> _isStopping;
public:
	std::string pipePath;
	std::string pipeName;
	std::thread *threadMain;
	std::mutex lockServer;

	class Client {
	private:
		std::atomic<bool> _isConnected;
	public:
		int id;
		HANDLE handlePipe;
		OVERLAPPED overlapped;
		NamedPipeServer *server;
		std::thread* threadRead;
		std::thread* threadWrite;
		std::string pipeName;
		std::mutex lockClient;
		BufferQueue writeQueue;
		std::mutex lockWriteWait;
		std::condition_variable conditionWriteQueue;
		Client();
		~Client();
		bool IsConnected();
		void SetConnectedState(bool connected);

		void RunReadThread();
		void RunWriteThread();
	};

	Client clients[MAX_CLIENTS];
	int clientCount;

	NamedPipeServer(std::string pipeName);
	~NamedPipeServer();

	virtual bool Start();
	virtual bool Stop();
	virtual void ProcessData(int clientId, BYTE *bufferInput, int length);
	bool Write(void * buffer, int length);
	bool IsClientConnected();

	void RunMainThread();



};

