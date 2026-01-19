#include "TcpServer.h"

TcpServer::TcpServer(int port) : port(port), serverSocket(INVALID_SOCKET), isRunning(false)
{}
TcpServer::~TcpServer() {
	Stop();
}
// Initialize Winsock and start the server
bool TcpServer::Start() {
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if(result != 0) {
		std::cerr << "WSAStartup failed: " << result << std::endl;
		return false;
	}
	// Create a TCP socket
	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(serverSocket == INVALID_SOCKET) {
		std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return false;
	}
	// Bind the socket to the specified port
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port);

	if(bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
		closesocket(serverSocket);
		WSACleanup();
		return false;
	}
	// Start listening for incoming connections
	if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
		std::cerr << "Listen failed: " << WSAGetLastError() << std::endl;
		closesocket(serverSocket);
		WSACleanup();
		return false;
	}
	isRunning = true;
	std::cout << "Server started on port " << port << std::endl;

	// Start the client listening thread
	std::thread(&TcpServer::ListenForClients, this).detach();

	return true;
}
// Stop the server and clean up resources
void TcpServer::Stop() {
	if(isRunning) {
		isRunning = false;
		closesocket(serverSocket);
		WSACleanup();

		if(serverThread.joinable()) {
			serverThread.join();
		}
		std::cout << "Server stopped." << std::endl;
	}
}
// Listen for incoming client connections
void TcpServer::ListenForClients() {
	while(isRunning) {
		sockaddr_in clientAddr;
		int clientAddrSize = sizeof(clientAddr);
		SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);

		if(clientSocket == INVALID_SOCKET) {
			if(isRunning){
				std::cerr << "Accept failed: " << WSAGetLastError() << std::endl;
			}
			continue;
		}
		std::cout << "\n[NETWORK] Client connected!" << std::endl;

		char buffer[1024];
		ZeroMemory(buffer, sizeof(buffer));

		int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

		if (bytesReceived > 0) {
			std::string message(buffer, bytesReceived);
			std::cout << "[NETWORK] Received message: " << message << std::endl;

			closesocket(clientSocket);
			std::cout << "Client disconnected." << std::endl;
		}
	}
}