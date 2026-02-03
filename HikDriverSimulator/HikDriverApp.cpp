#pragma once
#define NOMINMAX
#include "HikDriverApp.h"
#include <iostream>
#include <limits>
#include "AlarmService.h"
#include "Logger.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

HikDriverApp::HikDriverApp() : isRunning(true) {

	Logger::Init("applcation.log");
	Logger::Info("HikDriver Simulator started");

	alarmService.InitializeZones();
	alarmService.LoadStateFromJson();
	tcpServer = std::make_unique<TcpServer>(12345, &alarmService);

	if (!tcpServer->Start()) {
		Logger::Error("Failed to start TCP Server");
	}
}
HikDriverApp::~HikDriverApp() {
	Logger::Info("[APP] Shutting down HikDriver Simulator");
	alarmService.SaveStateToJson();
}

void HikDriverApp::ShowMenu() {
	std::cout << "\n================ HIKVISION DRIVER SIMULATOR ================" << std::endl;
	std::cout << "--- Controls ---" << std::endl;
	std::cout << "1. Arm Zone" << std::endl;
	std::cout << "2. Disarm Zone" << std::endl;
	std::cout << "3. Bypass Zone" << std::endl;
	std::cout << "4. Unbypass Zone" << std::endl;

	std::cout << "\n--- Monitoring / Lists ---" << std::endl;
	std::cout << "5. List All Zones" << std::endl;
	std::cout << "6. List ARMED Zones Only" << std::endl;
	std::cout << "7. List DISARMED Zones Only" << std::endl;
	std::cout << "8. List BYPASSED Zones Only" << std::endl;
	std::cout << "9. List ALARMING Zones Only" << std::endl;
	std::cout << "10. Find Zone by ID" << std::endl;

	std::cout << "0. Exit" << std::endl;
	std::cout << "Select option: ";
}
void HikDriverApp::Run() {

	int choice = -1;
	int id = 0;

	while (choice != 0) {
		ShowMenu();

		if (!(std::cin >> choice)) {
			std::cout << "ERROR: Please enter a valid number!" << std::endl;
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			continue;
		}

		std::cout << "\n";

		switch (choice) {
		case 1: // Arm
			std::cout << "Enter Zone ID to ARM: ";
			std::cin >> id;
			PrintJsonToConsole(alarmService.ArmZone(id));
			break;
		case 2: // Disarm
			std::cout << "Enter Zone ID to DISARM: ";
			std::cin >> id;
			PrintJsonToConsole(alarmService.DisarmZone(id));
			break;
		case 3: // Bypass
			std::cout << "Enter Zone ID to BYPASS: ";
			std::cin >> id;
			PrintJsonToConsole(alarmService.BypassZone(id, true));
			break;
		case 4: // Unbypass
			std::cout << "Enter Zone ID to UNBYPASS: ";
			std::cin >> id;
			PrintJsonToConsole(alarmService.BypassZone(id, false));
			break;
		case 5:
			PrintJsonToConsole(alarmService.ListAllZones());
			break;
		case 6:
			PrintJsonToConsole(alarmService.ListArmedZones());
			break;
		case 7:
			PrintJsonToConsole(alarmService.ListDisarmedZones());
			break;
		case 8:
			PrintJsonToConsole(alarmService.ListBypassedZones());
			break;
		case 9:
			PrintJsonToConsole(alarmService.ListAlarmingZones());
			break;
		case 10:
			std::cout << "Enter Zone ID to find: ";
			std::cin >> id;
			PrintJsonToConsole(alarmService.ListOneZone(id));
			break;
		case 0:
			std::cout << "Exiting system..." << std::endl;
			break;
		default:
			std::cout << "Unknown command. Please try again." << std::endl;
		}

		// Wait for user input before clearing or showing menu again
		if (choice != 0) {
			std::cout << "\n(Press Enter to continue...)";
			std::cin.ignore();
			std::cin.get();
		}
	}
}
void HikDriverApp::PrintJsonToConsole(const std::string& jsonResponse) {
	try
	{
		auto jsonInp = json::parse(jsonResponse);
		if (jsonInp.is_array()) {
			if (jsonInp.empty()) {
				Logger::Info("Zone not found");
				std::cout << "[INFO] Zone not found" << std::endl;
				return;
			}
			std::cout << "\n   --- ZONE LIST ---" << std::endl;
			for (const auto& item : jsonInp) {

				if (item.contains("id")) std::cout << "| ID: " << item["id"];
				if (item.contains("name")) std::cout << "| Name: " << item["name"];
				if (item.contains("type")) std::cout << "| Type: " << item["type"];
				if (item.contains("armed")) std::cout << " | Status: " << (item["armed"].get<bool>() ? "ARMED" : "DISARMED");
				if (item.contains("bypassed") && item["bypassed"].get<bool>()) std::cout << " | BYPASSED";
				if (item.contains("alarming") && item["alarming"].get<bool>()) std::cout << " | !!! ALARM !!!";
				std::cout << std::endl;;
			}
			std::cout << "   -----------------\n" << std::endl;
		}
		else {
			std::string status = jsonInp.value("status", "UNKNOWN");
			std::string message = jsonInp.value("message", "");

			if (status == "SUCCESS") {
				if (jsonInp.contains("id")) std::cout << "| ID: " << jsonInp["id"];
				if (jsonInp.contains("name")) std::cout << "| Name: " << jsonInp["name"];
				if (jsonInp.contains("type")) std::cout << "| Type: " << jsonInp["type"];
				if (jsonInp.contains("armed")) std::cout << " | Status: " << (jsonInp["armed"].get<bool>() ? "ARMED" : "DISARMED");
				if (jsonInp.contains("bypassed") && jsonInp["bypassed"].get<bool>()) std::cout << " | BYPASSED";
				if (jsonInp.contains("alarming") && jsonInp["alarming"].get<bool>()) std::cout << " | !!! ALARM !!!";
				if (jsonInp.contains("message")) std::cout << "| Message: " << jsonInp["message"];
				if (jsonInp.contains("newState")) std::cout << "| New Sate: " << jsonInp["newState"] << std::endl;;
			}
			else if (status == "ERROR" || status == "ALARM" || status == "IGNORED") {
				if (jsonInp.contains("id")) std::cout << "| ID: " << jsonInp["id"];
				if (jsonInp.contains("name")) std::cout << "| Name: " << jsonInp["name"];
				std::cout << " [" << status << "] ";
				std::cout << " [" << message << "] " << std::endl;
			}
			else {
				if (jsonInp.contains("id")) std::cout << "| ID: " << jsonInp["id"];
				std::cout << " [" << status << "] " << message << std::endl;
			}
		}
	}
	catch (const std::exception& e) {
		std::cout << " [RAW] " << jsonResponse << std::endl;
	}
}