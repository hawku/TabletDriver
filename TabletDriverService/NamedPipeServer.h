#pragma once

#include <thread>
#include <mutex>
#include <string>

#include "Runnable.h"

#define BUFFER_SIZE 1024

class NamedPipeServer : public Runnable
{
public:
	string pipePath;
	string pipeName;
	thread *threadMain;
	mutex lock;

	class Client {
	public:
		int id;
		HANDLE handlePipe;
		thread* threadPipe;
		string pipeName;
		bool isConnected;
		Client();
	};

	Client clients[256];
	int clientCount;

	NamedPipeServer(string pipeName);
	~NamedPipeServer();

	virtual bool Start();
	virtual bool Stop();
	virtual int ProcessData(int clientId, char *bufferInput, int length, char *bufferOutput);
	bool Write(void * buffer, int length);

	void RunMainThread();
	void RunClientThread(Client *client);



};

