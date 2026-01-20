#pragma once
#include <vector>
#include <iostream>
#include <string>
#include <thread>
#include <ws2tcpip.h>
#include <winsock2.h>
#include "AlarmService.h"
#pragma comment(lib, "ws2_32.lib")

class TcpServer {
private:
	SOCKET serverSocket;
	int port;
	bool isRunning;
	std::thread serverThread;
	AlarmService* alarmService;

	void ListenForClients();
	void SendResponse(SOCKET clientSocket, std::string& response);

public:
	TcpServer(int port);
	TcpServer(int port, AlarmService* alarmService);
	~TcpServer();
	bool Start();
	void Stop();


};

