#include "TcpServer.h"
#include <ranges>
#include <algorithm>
#include <cctype>
#include <string>
#include "Logger.h"


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
		Logger::Error("WSAStartup failed: " + std::to_string(result));
		return false;
	}
	// Create a TCP socket
	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET) {
		Logger::Error("Socket creation failed: " + std::to_string(WSAGetLastError()));
		WSACleanup();
		return false;
	}
	// Bind the socket to the specified port
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port);

	if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		Logger::Error("Bind failed: " + std::to_string(WSAGetLastError()));
		closesocket(serverSocket);
		WSACleanup();
		return false;
	}
	// Start listening for incoming connections
	if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
		Logger::Error("Listen failed: " + std::to_string(WSAGetLastError()));
		closesocket(serverSocket);
		WSACleanup();
		return false;
	}
	isRunning = true;
	Logger::Network("TCP Server started on port " + std::to_string(port));

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
		Logger::Network("TCP Server stopped.");
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
				Logger::Error("Accept failed: " + std::to_string(WSAGetLastError()));
			}
			continue;
		}
		Logger::Network("Client connected from ");
		
		char buffer[1024];
		ZeroMemory(buffer, sizeof(buffer));

		int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

		if (bytesReceived > 0) {
			std::string message(buffer, bytesReceived);
			Logger::Network("Received message: " + message);

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
							Logger::Info("Zone " + std::to_string(zoneId) + " Armed via Network.");
						}
						else {
							Logger::Warning("Zone " + std::to_string(zoneId) + " is already Armed.");
							response = "FAILED: Zone " + std::to_string(zoneId) + " is already Armed.";
						}
					}
					else if (command == "DISARM") {
						bool isArmed = alarmService->GetZoneById(zoneId)->isArmed;
						if (!isArmed) {
							Logger::Warning("Zone " + std::to_string(zoneId) + " is already Disarmed.");
							response = "FAILED: Zone " + std::to_string(zoneId) + " is already Disarmed.";
						}
						else {
							alarmService->DisarmZone(zoneId);
							Logger::Info("Zone " + std::to_string(zoneId) + " Disarmed via Network.");
						}
					}
					else if (command == "BYPASS") {
						alarmService->BypassZone(zoneId, true);
						Logger::Info("Zone " + std::to_string(zoneId) + " Bypassed via Network.");
					}
					else if (command == "UNBYPASS") {
						alarmService->BypassZone(zoneId, false);
						Logger::Info("Zone " + std::to_string(zoneId) + " Unbypassed via Network.");
					}
					else if (command == "STATUS") {
						std::string status = alarmService->GetZoneStatus(zoneId);

						if (status == "NOT_FOUND") {
							Logger::Error("Status query failed: Zone " + std::to_string(zoneId) + " not found.");
						}
						else {
							response = "STATUS:" + status;
							Logger::Info("Zone " + std::to_string(zoneId) + " status queried: " + status);
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
						Logger::Info("Trigger signal processed for Zone " + std::to_string(zoneId));
					}
					else {
						response = "ERROR: Unknown command";
						Logger::Error("Unknown command received: " + command);
					}
				}
				catch (const std::exception&)
				{
					response = "ERROR: Invalid ID format";
					Logger::Error("Invalid ID format received.");
				}
			}
			else {
				Logger::Error("Invalid message format received.");
			}

			SendResponse(clientSocket, response);
			Logger::Network("Response sent: " + response);
			closesocket(clientSocket);
			Logger::Network("Client disconnected.");
		}
	}
}
// Send a response back to the client
void TcpServer::SendResponse(SOCKET clientSocket, std::string& response) {
	response += "\n";
	int bytesSent = send(clientSocket, response.c_str(), response.length(), 0);
	if (bytesSent == SOCKET_ERROR) {
		Logger::Error("Send failed: " + std::to_string(WSAGetLastError()));
	}
}