#include "precompiled.h"
#include "NamedPipeServer.h"

#define LOG_MODULE "NamedPipeServer"
#include "Logger.h"


//
// Constructor
//
NamedPipeServer::NamedPipeServer(std::string pipeName)
{
	this->pipePath = "\\\\.\\PIPE\\" + pipeName;
	this->pipeName = pipeName;
	clientCount = 0;
	for (int i = 0; i < MAX_CLIENTS; i++) {
		clients[i].id = i;
		clients[i].pipeName = pipeName;
	}
	_isStopping = false;

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
	if (!IsRunning()) {
		SetRunningState(true);

		LOG_DEBUG("Starting %s main thread...\n", pipeName.c_str());
		threadMain = new std::thread(&NamedPipeServer::RunMainThread, this);
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
	_isStopping = true;

	SetRunningState(false);

	printf("%s: Stopping main thread...\n", pipeName.c_str());

	printf("%s: Creating temporary client...\n", pipeName.c_str());
	HANDLE tmpHandle = CreateFileA(pipePath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	DWORD lastError = GetLastError();
	if (tmpHandle != INVALID_HANDLE_VALUE)
	{
		DWORD bytesWritten, mode;
		BYTE buffer[2];
		mode = PIPE_READMODE_MESSAGE;
		SetNamedPipeHandleState(tmpHandle, &mode, NULL, NULL);
		buffer[0] = '\n';
		buffer[1] = 0;
		WriteFile(tmpHandle, buffer, 2, &bytesWritten, NULL);
		FlushFileBuffers(tmpHandle);
		SAFE_CLOSE_HANDLE(tmpHandle);
		printf("%s: Temporary created and disconnected!\n", pipeName.c_str());
	}
	else if (lastError == ERROR_FILE_NOT_FOUND) {
		printf("%s: Temporary client creation failed! File not found! GLE=%d\n", pipeName.c_str(), lastError);
	}
	else if (lastError == ERROR_SHARING_VIOLATION) {
		printf("%s: Temporary client creation failed! Sharing violation! GLE=%d\n", pipeName.c_str(), lastError);
	}
	else {
		printf("%s: Temporary client creation failed! GLE=%d\n", pipeName.c_str(), lastError);
	}

	// Force disconnect every client
	lockServer.lock();
	for (int i = 0; i < MAX_CLIENTS; i++) {

		clients[i].SetConnectedState(false);
	}
	lockServer.unlock();


	// Join main thread
	printf("%s: Joining main thread...\n", pipeName.c_str());
	try {
		threadMain->join();
	}
	catch (std::exception &e) {
		printf("%s: main thread exception: %s\n", pipeName.c_str(), e.what());
	}
	printf("%s: Main thread closed!\n", pipeName.c_str());

	// Join client threads
	printf("%s: Stopping clients...\n", pipeName.c_str());

	for (int i = 0; i < clientCount; i++) {

		//Sleep(1);

		if (clients[i].handlePipe != NULL) {
			CancelSynchronousIo(clients[i].handlePipe);
			CancelIoEx(clients[i].handlePipe, &clients[i].overlapped);
			DisconnectNamedPipe(clients[i].handlePipe);
		}
		//SAFE_CLOSE_HANDLE(clients[i].handlePipe);


		// Read thread join
		printf("  %s: Client #%d read thread join\n", pipeName.c_str(), i);
		if (clients[i].threadRead != NULL) {

			// Try thread join
			try {
				if (clients[i].threadRead != NULL) {
					clients[i].threadRead->join();
				}
			}
			catch (std::exception &e) {
				printf("   %s: read thread #%d exception: %s\n", pipeName.c_str(), i, e.what());
			}

		}

		// Write thread join
		printf("  %s: Client #%d write thread join\n", pipeName.c_str(), i);
		if (clients[i].threadWrite != NULL) {

			// Notify queue change
			for (int j = 0; j < 10; j++) {
				clients[i].writeQueue.Notify();
			}

			// Try thread join
			try {
				if (clients[i].threadWrite != NULL) {
					clients[i].writeQueue.Clear();
					clients[i].threadWrite->join();
				}
			}
			catch (std::exception &e) {
				printf("   %s: write thread #%d exception: %s\n", pipeName.c_str(), i, e.what());
			}

		}
		printf("%s: Clients stopped!\n", pipeName.c_str());

	}
	return true;
}


//
// Process data
//
void NamedPipeServer::ProcessData(int clientId, BYTE *bufferInput, int length) {
}


//
// Add the data to every client write buffer
//
bool NamedPipeServer::Write(void *buffer, int length) {

	if (_isStopping) return false;
	if (!IsRunning()) return false;

	// Validate length
	if (length <= 0) return false;
	if (length >= BUFFER_SIZE) {
		length = BUFFER_SIZE;
		((char*)buffer)[length - 1] = 0;
		((char*)buffer)[length - 2] = '\n';
	}

	// Add to queue
	for (int i = 0; i < clientCount; i++) {
		if (clients[i].IsConnected()) {
			Client *client = &clients[i];
			client->writeQueue.Push(buffer, length);
		}
	}

	return true;

}

//
// Is client connected?
//
bool NamedPipeServer::IsClientConnected()
{
	if (_isStopping) return true;

	lockServer.lock();
	bool connected = false;
	for (int i = 0; i < clientCount; i++) {
		if (clients[i].IsConnected())
			connected = true;
	}
	lockServer.unlock();
	return connected;
}


//
// Main thread
//
void NamedPipeServer::RunMainThread()
{
	bool connected;
	bool running = true;
	Client *client = NULL;
	DWORD lastError = 0;
	int clientIndex = 0;
	HANDLE tmpHandle;

	while (IsRunning()) {

		if (_isStopping) break;

		// Create pipe
		LOG_DEBUG("%s: Creating pipe...\n", pipeName.c_str());
		tmpHandle = CreateNamedPipeA(pipePath.c_str(),
			PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES,
			BUFFER_SIZE,
			BUFFER_SIZE,
			100,
			NULL
		);
		// Pipe invalid?
		if (tmpHandle == INVALID_HANDLE_VALUE)
		{
			lastError = GetLastError();
			LOG_ERROR("%s: CreateNamedPipeA failed, GLE=%lu\n", pipeName.c_str(), lastError);
			//Sleep(100);
			break;
		}
		LOG_DEBUG("Pipe %s created!\n", pipeName.c_str());


		// Connect
		LOG_DEBUG("%s: Connecting to pipe...\n", pipeName.c_str());
		connected = ConnectNamedPipe(tmpHandle, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

		// Connected?
		if (connected) {

			/*
			// Cleanup the temporary client when the server is stopping
			if (_isStopping) {
				DisconnectNamedPipe(tmpHandle);
				SAFE_CLOSE_HANDLE(tmpHandle);
				break;
			}
			*/

			//
			// Find free client id
			//
			client = &clients[clientIndex];
			if (
				client->IsConnected()
				||
				client->threadRead != NULL
				||
				client->threadWrite != NULL
				) {
				int index = -1;

				// Find empty slot
				for (int i = 0; i < MAX_CLIENTS; i++) {
					if (
						!clients[i].IsConnected()
						&&
						clients[i].threadRead == NULL
						&&
						clients[i].threadWrite == NULL
						) {
						index = i;
						break;
					}
				}

				if (index >= 0 && index < MAX_CLIENTS) {
					clientIndex = index;
					client = &clients[clientIndex];
				}
				else {
					LOG_ERROR("%s: Couldn't find a free client slot!\n", pipeName.c_str());
					DisconnectNamedPipe(tmpHandle);
					SAFE_CLOSE_HANDLE(tmpHandle);
					//Sleep(2000);
					client = NULL;
				}
			}
			else {

			}

			// Skip when there are no free client slots
			if (client == NULL) {
				continue;
			}
			else {
				client->handlePipe = tmpHandle;
			}


			LOG_DEBUG("%s: Client #%d connected!\n", pipeName.c_str(), clientIndex);

			client->writeQueue.Clear();
			client->SetConnectedState(true);

			// Cleanup the temporary client when the server is stopping
			if (_isStopping) {
				DisconnectNamedPipe(tmpHandle);
				SAFE_CLOSE_HANDLE(tmpHandle);
				break;
			}

			client->server = this;
			client->threadRead = new std::thread([&] { client->RunReadThread(); });
			client->threadWrite = new std::thread([&] { client->RunWriteThread(); });
			clientIndex++;
			if (clientIndex >= MAX_CLIENTS) {
				clientIndex = 0;
			}
			if (clientIndex >= clientCount - 1) {
				clientCount = clientIndex + 1;
			}

			clientIndex = 0;


			if (clientCount >= MAX_CLIENTS) {
				clientCount = MAX_CLIENTS;
			}
		}

		// No connection
		else {
			LOG_DEBUG("%s: Could not connect to a pipe!\n", pipeName.c_str());
			SAFE_CLOSE_HANDLE(tmpHandle);
			//Sleep(100);
		}

	}

}


//
// Client read thread
//
void NamedPipeServer::Client::RunReadThread()
{
	BYTE bufferRead[BUFFER_SIZE];
	DWORD bytesRead = 0;
	DWORD bytesAvailable = 0;
	DWORD bytesLeft = 0;
	bool result;
	DWORD lastError;

	// Loop
	while (true) {

		if (!server->IsRunning()) {
			break;
		}

		if (server->_isStopping) break;

		// Is not connected?
		if (!IsConnected()) {
			break;
		}

		// Read data from pipe
		bytesRead = 0;
		result = ReadFile(handlePipe, bufferRead, BUFFER_SIZE, &bytesRead, NULL);

		if (!result || bytesRead == 0) {
			lastError = GetLastError();
			if (lastError == ERROR_BROKEN_PIPE)
			{
				LOG_DEBUG("%s: Client #%d read broken pipe!\n", pipeName.c_str(), id);
				break;
			}
			else if (lastError != 0)
			{
				// Skip overlapping error
				if (lastError == ERROR_INVALID_PARAMETER) {
				}
				else {
					LOG_DEBUG("%s: Client #%d ReadFile failed, GLE=%lu\n", pipeName.c_str(), id, lastError);
					break;
				}
			}
			//Sleep(10);
		}
		else {
			server->ProcessData(id, bufferRead, bytesRead);
		}

	}

	// Cleanup
	if (server != NULL && !server->_isStopping) {
		printf("%s: Client #%d read thread cleanup!\n", pipeName.c_str(), id);
		SetConnectedState(false);
		CancelIoEx(handlePipe, &overlapped);
		writeQueue.Notify();
		lockClient.lock();
		SAFE_CLOSE_HANDLE(handlePipe);
		this->threadRead = NULL;
		lockClient.unlock();
	}
	printf("%s: Client #%d read thread exit!\n", pipeName.c_str(), id);

}

//
// Client write thread
//
void NamedPipeServer::Client::RunWriteThread()
{
	DWORD bytesWritten = 0;
	bool result;
	DWORD lastError;
	bool running = true;


	// Loop
	while (running) {

		if (!server->IsRunning()) {
			break;
		}

		if (server->_isStopping) break;

		// Is not connected?
		if (!IsConnected()) {
			break;
		}

		// Wait for a new queue item
		writeQueue.Wait();


		// Items in write queue?
		while (!writeQueue.IsEmpty()) {

			// Lock queue
			if (writeQueue.lockQueue.try_lock()) {

				// Get queue item
				BufferQueue::Item *queueItem = writeQueue.Front();

				// Copy data
				int length = queueItem->length;
				BYTE *buffer = new BYTE[length];
				memcpy(buffer, queueItem->buffer, length);

				// Remove queue item
				writeQueue.Pop();

				// Unlock queue
				writeQueue.lockQueue.unlock();


				// Write item to pipe
				result = false;
				try {
					result = WriteFile(handlePipe, buffer, length, NULL, &overlapped);
				}
				catch (std::exception &e) {
					LOG_ERROR("WriteFile exception: %s\n", e.what());
				}

				// Destroy buffer
				delete buffer;

				// Write error?
				if (!result)
				{
					lastError = GetLastError();

					// Broken pipe
					if (lastError == ERROR_BROKEN_PIPE)
					{
						LOG_DEBUG("%s: Client #%d write broken pipe!\n", pipeName.c_str(), id);
						running = false;
						break;
					}

					// Skip overlapping error
					else if (lastError == ERROR_INVALID_PARAMETER) {
					}
					else {
						LOG_DEBUG("%s: Client #%d WriteFile failed, GLE=%lu\n", pipeName.c_str(), id, lastError);
						running = false;
						break;
					}
				}

				// Flush file buffers
				else {
					FlushFileBuffers(handlePipe);
				}
			}
			else {
				printf("%s: Write queue lock failed !!!\n", pipeName.c_str());
			}
		}


	}

	// Cleanup
	if (server != NULL && !server->_isStopping) {
		printf("%s: Client #%d write thread cleanup!\n", pipeName.c_str(), id);
		SetConnectedState(false);
		CancelIoEx(handlePipe, &overlapped);
		lockClient.lock();
		writeQueue.Clear();
		SAFE_CLOSE_HANDLE(handlePipe);
		this->threadWrite = NULL;
		lockClient.unlock();
	}
	printf("%s: Client #%d write thread exit!\n", pipeName.c_str(), id);


}



//
// Client constructor
//
NamedPipeServer::Client::Client()
{
	id = 0;
	handlePipe = NULL;
	threadRead = NULL;
	threadWrite = NULL;
	_isConnected = false;
	pipeName = "";
	memset(&overlapped, 0, sizeof(overlapped));
	overlapped.Offset = 4096;
	overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
}

//
// Client destructor
//
NamedPipeServer::Client::~Client()
{
}

//
// Check if client is connected
//
bool NamedPipeServer::Client::IsConnected()
{
	std::unique_lock<std::mutex> mlock(lockClient);
	return _isConnected;
}

//
// Set client connected state
//
void NamedPipeServer::Client::SetConnectedState(bool connected)
{
	std::unique_lock<std::mutex> mlock(lockClient);
	_isConnected = connected;
}
