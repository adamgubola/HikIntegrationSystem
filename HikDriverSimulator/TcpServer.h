#pragma once
#include <vector>
#include <iostream>
#include <string>
#include <thread>
#include <ws2tcpip.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

class TcpServer {
private:
	SOCKET serverSocket;
	int port;
	bool isRunning;

public:
	TcpServer(int port);
	~TcpServer();
	bool Start();
	void Stop();
	void ListenForClients();

};

