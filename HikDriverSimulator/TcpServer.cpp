#include "TcpServer.h"
#include <ranges>
#include <algorithm>
#include <cctype>
#include <string>


TcpServer::TcpServer(int port) : port(port), serverSocket(INVALID_SOCKET), isRunning(false)
{
}

TcpServer::TcpServer(int port, AlarmService* alarmService) : port(port), alarmService(alarmService), serverSocket(INVALID_SOCKET), isRunning(false)
{
}

TcpServer::~TcpServer() {
	Stop();
}
// Initialize Winsock and start the server
bool TcpServer::Start() {
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (result != 0) {
		std::cerr << "WSAStartup failed: " << result << std::endl;
		return false;
	}
	// Create a TCP socket
	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET) {
		std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return false;
	}
	// Bind the socket to the specified port
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port);

	if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
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
	if (isRunning) {
		isRunning = false;
		closesocket(serverSocket);
		WSACleanup();

		if (serverThread.joinable()) {
			serverThread.join();
		}
		std::cout << "Server stopped." << std::endl;
	}
}
// Listen for incoming client connections
void TcpServer::ListenForClients() {
	while (isRunning) {
		sockaddr_in clientAddr;
		int clientAddrSize = sizeof(clientAddr);
		SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);

		if (clientSocket == INVALID_SOCKET) {
			if (isRunning) {
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

			std::string response = "ERROR: Unknown command format";

			size_t delimiterPos = message.find(':');

			if (delimiterPos != std::string::npos) {
				std::string command = message.substr(0, delimiterPos);
				std::transform(command.begin(), command.end(), command.begin(),
					[](auto c) { return std::toupper(c); });
				std::string paramStr = message.substr(delimiterPos + 1);
				try
				{
					int zoneId = std::stoi(paramStr);

					if (command == "ARM") {
						bool isArmed = alarmService->GetZoneById(zoneId)->isArmed;
						if (!isArmed) {
							alarmService->ArmZone(zoneId);
							response = "SUCCESS: Zone " + std::to_string(zoneId) + " Armed via Network.";
							std::cout << "[EXECUTE] Zone " << zoneId << " Armed via Network." << std::endl;
						}
						else {
							std::cout << "[EXECUTE] Zone " << zoneId << " is already Armed." << std::endl;
							response = "FAILED: Zone " + std::to_string(zoneId) + " is already Armed.";
						}
					}
					else if (command == "DISARM") {
						bool isArmed = alarmService->GetZoneById(zoneId)->isArmed;
						if (!isArmed) {
							std::cout << "[EXECUTE] Zone " << zoneId << " is already Disarmed." << std::endl;
							response = "FAILED: Zone " + std::to_string(zoneId) + " is already Disarmed.";
						}
						else {
							alarmService->DisarmZone(zoneId);
							response = "SUCCESS: Zone " + std::to_string(zoneId) + " Disarmed via Network.";
							std::cout << "[EXECUTE] Zone " << zoneId << " Disarmed via Network." << std::endl;
						}
					}
					else if (command == "BYPASS") {
						alarmService->BypassZone(zoneId, true);
						response = "SUCCESS: Zone " + std::to_string(zoneId) + " Bypassed via Network.";
						std::cout << "[EXECUTE] Zone " << zoneId << " Bypassed via Network." << std::endl;
					}
					else if (command == "UNBYPASS") {
						alarmService->BypassZone(zoneId, false);
						response = "SUCCESS: Zone " + std::to_string(zoneId) + " Unbypassed via Network.";
						std::cout << "[EXECUTE] Zone " << zoneId << " Unbypassed via Network." << std::endl;
					}
					else if (command == "STATUS") {
						std::string status = alarmService->GetZoneStatus(zoneId);

						if (status == "NOT_FOUND") {
							response = "ERROR: Zone " + std::to_string(zoneId) + " not found";
							std::cout << "[ERROR] Status query failed: Zone " << zoneId << " not found." << std::endl;
						}
						else {
							response = "STATUS:" + status;
							std::cout << "[QUERY] Zone " << zoneId << " status sent: " << status << std::endl;
						}
					}
					else if (command == "TRIGGER") {
						alarmService->TriggerZone(zoneId);

						auto zone = alarmService->GetZoneById(zoneId);
						if (zone && zone->isAlarming) {
							response = "ALARM: Zone " + std::to_string(zoneId) + " went into ALARM state!";
						}
						else {
							response = "OK: Zone " + std::to_string(zoneId) + " triggered (no alarm generated)";
						}
						std::cout << "[EXECUTE] Trigger signal processed for Zone " << zoneId << std::endl;
					}
					else {
						response = "ERROR: Unknown command";
						std::cout << "[NETWORK] Unknown command: " << command << std::endl;
					}
				}
				catch (const std::exception&)
				{
					response = "ERROR: Invalid ID format";
					std::cout << "[ERROR] Invalid ID format." << std::endl;
				}
			}
			else {
				std::cout << "[ERROR] Invalid message format. Use 'Command:ID'" << std::endl;
			}

			SendResponse(clientSocket, response);
			std::cout << "[NETWORK] Response sent: " << response << std::endl;
			closesocket(clientSocket);
			std::cout << "Client disconnected." << std::endl;
		}
	}
}
// Send a response back to the client
void TcpServer::SendResponse(SOCKET clientSocket, std::string& response) {
	response += "\n";
	int bytesSent = send(clientSocket, response.c_str(), response.length(), 0);
	if (bytesSent == SOCKET_ERROR) {
		std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
	}
}