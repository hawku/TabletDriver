#include "stdafx.h"
#include "NamedPipeServer.h"

#define LOG_MODULE "NamedPipeServer"
#include "Logger.h"


//
// Constructor
//
NamedPipeServer::NamedPipeServer(string pipeName)
{
	this->pipePath = "\\\\.\\PIPE\\" + pipeName;
	this->pipeName = pipeName;
	clientCount = 0;

	for(int i = 0; i < 256; i++) {
		clients[i].id = i;
		clients[i].pipeName = pipeName;
	}

}


//
// Destructor
//
NamedPipeServer::~NamedPipeServer()
{
	Stop();
}

//
// Start pipe server
//
bool NamedPipeServer::Start()
{
	if(!IsRunning()) {
		SetRunningState(true);

		LOG_DEBUG("Starting %s main thread...\n", pipeName.c_str());
		threadMain = new thread(&NamedPipeServer::RunMainThread, this);
		LOG_DEBUG("%s main thread started!\n", pipeName.c_str());

		return true;
	}
	return false;
}


//
// Stop pipe server
//
bool NamedPipeServer::Stop()
{
	if(IsRunning()) {
		SetRunningState(false);
	}
	return false;
}


//
// Process data
//
int NamedPipeServer::ProcessData(int clientId, char *bufferInput, int length, char *bufferOutput) {
	int bytesOutput = 0;

	//bufferInput[length] = 0;
	//LOG_DEBUG("Client #%d input: %s\n", clientId, bufferInput);
	//bytesOutput = sprintf_s(bufferOutput, BUFFER_SIZE, "Reply: %s", bufferInput);

	return bytesOutput;
}


//
// Write a message to all clients
//
bool NamedPipeServer::Write(void * buffer, int length) {

	DWORD bytesWritten = 0;

	if(!IsRunning()) return false;

	for(int i = 0; i < clientCount; i++) {
		if(clients[i].isConnected) {
			Client *client = &clients[i];
			//printf("Write %d bytes to client #%d!\n", length, i);
			WriteFile(client->handlePipe, buffer, length, &bytesWritten, NULL);
			FlushFileBuffers(client->handlePipe);
			//printf("Written %d bytes to client #%d!\n", bytesWritten, i);
		}
	}
	return true;

}


//
// Main thread
//
void NamedPipeServer::RunMainThread()
{
	bool connected;
	int clientId = 0;
	bool running = true;
	Client *client;

	while(IsRunning()) {

		LOG_DEBUG("%s: Creating pipe...\n", pipeName.c_str());

		// Find free client id
		client = &clients[clientId];

		// Create pipe
		client->handlePipe = CreateNamedPipeA(pipePath.c_str(),
			PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES,
			BUFFER_SIZE,
			BUFFER_SIZE,
			100,
			NULL
		);
		// Pipe invalid?
		if(client->handlePipe == INVALID_HANDLE_VALUE)
		{
			LOG_ERROR("%s: CreateNamedPipeA failed, GLE=%d\n", pipeName.c_str(), GetLastError());
			Sleep(100);
			break;
		}

		LOG_DEBUG("Pipe %s created!\n", pipeName.c_str());


		// Connect
		LOG_DEBUG("%s: Connecting to pipe...\n", pipeName.c_str());
		connected = ConnectNamedPipe(client->handlePipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

		// Connected?
		if(connected) {
			LOG_DEBUG("%s: Client connected!\n", pipeName.c_str());
			thread *threadPipe = new thread(&NamedPipeServer::RunClientThread, this, client);
			client->threadPipe = threadPipe;
			clientCount++;
			if(clientCount >= 256) {
				clientCount = 256;
			}
			clientId++;
			if(clientId >= 256) {
				clientId = 0;
			}
		}

		// No connection
		else {
			LOG_DEBUG("%s: Could not connect to a pipe!\n", pipeName.c_str());
			SAFE_CLOSE_HANDLE(client->handlePipe);
		}

	}

}


//
// Client thread
//
void NamedPipeServer::RunClientThread(Client *client)
{
	char bufferRead[BUFFER_SIZE];
	char bufferWrite[BUFFER_SIZE];
	DWORD bytesRead = 0;
	DWORD bytesWritten = 0;
	int replyBytes;
	bool result;
	string pipeName = client->pipeName;
	HANDLE handlePipe = client->handlePipe;
	int clientId = client->id;
	client->isConnected = true;

	while(true) {
		//LOG_DEBUG("Read %s!\n", pipeName.c_str());

		// Try reading
		result = ReadFile(handlePipe, bufferRead, BUFFER_SIZE, &bytesRead, NULL);
		if(!result || bytesRead == 0) {
			if(GetLastError() == ERROR_BROKEN_PIPE)
			{
				LOG_DEBUG("%s: Client #%d disconnected.\n", pipeName.c_str(), clientId);
				break;
			}
			else if(GetLastError() != 0)
			{
				LOG_DEBUG("%s: Client #%d ReadFile failed, GLE=%d\n", pipeName.c_str(), clientId, GetLastError());
				break;
			}
			Sleep(10);
		}
		else {
			replyBytes = ProcessData(clientId, bufferRead, bytesRead, bufferWrite);

			if(replyBytes > 0) {
				result = WriteFile(handlePipe, bufferWrite, replyBytes, &bytesWritten, NULL);
				if(!result || replyBytes != bytesWritten)
				{
					LOG_DEBUG("%s: Client #%d WriteFile failed, GLE=%d\n", pipeName.c_str(), clientId, GetLastError());
					break;
				}
			}
		}

	}

	DisconnectNamedPipe(handlePipe);
	SAFE_CLOSE_HANDLE(handlePipe);
	if(client != nullptr && client != NULL) {
		client->isConnected = false;
	}
	LOG_DEBUG("%s: Client #%d thread exit!\n", pipeName.c_str(), clientId);

}


//
// Client constructor
//
NamedPipeServer::Client::Client()
{
	id = 0;
	handlePipe = NULL;
	threadPipe = NULL;
	isConnected = false;
}
