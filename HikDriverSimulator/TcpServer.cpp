#include "TcpServer.h"
#include <ranges>
#include <algorithm>
#include <cctype>
#include <string>
#include <ws2tcpip.h>
#include "Logger.h"
#include <nlohmann/json.hpp>


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
		char clientIp[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIp, INET_ADDRSTRLEN);
		Logger::Network("Client connected from " + std::string(clientIp));

		char buffer[1024];
		ZeroMemory(buffer, sizeof(buffer));

		int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

		if (bytesReceived > 0) {
			std::string message(buffer, bytesReceived);
			Logger::Network("Received message: " + message);

			std::string response = "ERROR: Unknown command format";

			std::string command;
			std::string paramStr;

			size_t delimiterPos = message.find(':');

			if (delimiterPos != std::string::npos) {
				command = message.substr(0, delimiterPos);
				paramStr = message.substr(delimiterPos + 1);
			}
			else {
				command = message;
				command.erase(std::remove(command.begin(), command.end(), '\n'), command.end());
				command.erase(std::remove(command.begin(), command.end(), '\r'), command.end());
			}
			std::transform(command.begin(), command.end(), command.begin(),
				[](auto c) { return std::toupper(c); });

			try {
				if (command == "ARM") {
					response = alarmService->ArmZone(std::stoi(paramStr));
				}
				else if (command == "DISARM") {
					response = alarmService->DisarmZone(std::stoi(paramStr));
				}
				else if (command == "BYPASS") {
					response = alarmService->BypassZone(std::stoi(paramStr), true);
				}
				else if (command == "UNBYPASS") {
					response = alarmService->BypassZone(std::stoi(paramStr), false);
				}
				else if (command == "STATUS") {
					response = alarmService->GetZoneStatus(std::stoi(paramStr));
				}
				else if (command == "TRIGGER") {
					response = alarmService->TriggerZone(std::stoi(paramStr));
				}
				else if (command == "LIST_ALL_ZONES") {
					response = alarmService->ListAllZones();
					Logger::Info("Sent list: All Zones");
				}
				else if (command == "LIST_ARMED_ZONES") {
					response = alarmService->ListArmedZones();
				}
				else if (command == "LIST_BYPASSED_ZONES") {
					response = alarmService->ListBypassedZones();
				}
				else if (command == "LIST_DISARMED_ZONES") {
					response = alarmService->ListDisarmedZones();
				}
				else if (command == "LIST_ALARMING_ZONES") {
					response = alarmService->ListAlarmingZones();
				}
				else if (command == "LIST_ONE_ZONE") {
					response = alarmService->ListOneZone(std::stoi(paramStr));
				}
				else if (command == "DISARM_PARTITION") {
					response = alarmService->DisarmPartition(std::stoi(paramStr));
				}
				else if (command == "ARM_PARTITION") {
					response = alarmService->ArmPartition(std::stoi(paramStr));
				}
				else {
					nlohmann::json jErr;
					jErr["status"] = "ERROR";
					jErr["message"] = "Unknown command: " + command;
					response = jErr.dump();
					Logger::Error("Unknown command: " + command);
				}
			}
			catch (const std::exception& e) {
				nlohmann::json jErr;
				jErr["status"] = "ERROR";
				jErr["message"] = "Invalid command format or ID";
				response = jErr.dump();
				Logger::Error("Exception in command processing: " + std::string(e.what()));
			}

			SendResponse(clientSocket, response);
			Logger::Network("Response sent: " + response);
			closesocket(clientSocket);
			Logger::Network("Client disconnected.");
		}
		else {
			Logger::Error("Invalid message format received.");
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