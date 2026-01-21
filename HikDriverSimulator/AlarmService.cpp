#include "AlarmService.h"
#include "MotionSenzor.h"
#include "DoorContact.h"
#include "Zone.h"
#include <iostream>
#include <fstream>
#include <sstream>

AlarmService::AlarmService() {}
AlarmService::~AlarmService() {}

// Initialize zones from zones.csv
void AlarmService::InitializeZones()
{
	std::cout << "Initializing zones from zone.csv" << std::endl;
	zones.clear();
	std::ifstream file("zones.csv");
	if (!file.is_open())
	{
		std::cerr << "Failed to open zones.csv" << std::endl;
		return;
	}
	std::string line;

	while (std::getline(file, line)) {
		if (line.empty()) continue;

		std::stringstream ss(line);
		std::string segment;
		std::vector<std::string> parts;

		while (std::getline(ss, segment, ';')) {
			parts.push_back(segment);
		}
		if (parts.size() >= 3)
		{
			int zoneId = std::stoi(parts[0]);
			std::string zoneType = parts[1];
			std::string zoneName = parts[2];

			std::shared_ptr<Zone> newZone;
			if (zoneType == "Motion Sensor")
			{
				newZone = std::make_shared<MotionSenzor>(zoneId, zoneName);
			}
			else if (zoneType == "Door Contact")
			{
				newZone = std::make_shared<DoorContact>(zoneId, zoneName);
			}
			else
			{
				newZone = std::make_shared<Zone>(zoneId, zoneName);
			}
			zones.push_back(newZone);
			std::cout << "Added zone: ID=" << zoneId << ", Name=" << zoneName << ", Type=" << zoneType << std::endl;
		}
		else {
			std::cerr << "Invalid line in zones.csv: " << line << std::endl;
		}

	}
	file.close();
	std::cout << zones.size() << " zones initialized." << std::endl;

}
void AlarmService::ArmZone(int zoneId)
{
	for (auto& zone : zones)
	{
		if (zone->id == zoneId)
		{
			zone->Arm();
			return;
		}
	}
	std::cout << "Zone with ID " << zoneId << " not found." << std::endl;

}
void AlarmService::DisarmZone(int zoneId)
{
	for (auto& zone : zones)
	{
		if (zone->id == zoneId)
		{
			zone->Disarm();
			return;
		}
	}
	std::cout << "Zone with ID " << zoneId << " not found." << std::endl;
}
void AlarmService::BypassZone(int zoneId, bool active)
{
	for (auto& zone : zones)
	{
		if (zone->id == zoneId)
		{
			zone->SetBypass(active);
			return;
		}
	}
	std::cout << "Zone with ID " << zoneId << " not found." << std::endl;
}
void AlarmService::ListAllZones()
{
	std::cout << "Listing all zones:" << std::endl;
	for (const auto& zone : zones)
	{
		std::cout << "ID: " << zone->id
			<< ", Name: " << zone->name
			<< ", Type: " << zone->GetType()
			<< ", Armed: " << (zone->isArmed ? "Yes" : "No")
			<< ", Alarming: " << (zone->isAlarming ? "Yes" : "No")
			<< ", Bypassed: " << (zone->isBypassed ? "Yes" : "No")
			<< std::endl;
	}
	if (zones.empty())
	{
		std::cout << "No zones available." << std::endl;
	}
}
void AlarmService::ListOneZone(int zoneId)
{
	for (const auto& zone : zones)
	{
		if (zone->id == zoneId)
		{
			std::cout << "ID: " << zone->id
				<< ", Name: " << zone->name
				<< ", Type: " << zone->GetType()
				<< ", Armed: " << (zone->isArmed ? "Yes" : "No")
				<< ", Alarming: " << (zone->isAlarming ? "Yes" : "No")
				<< ", Bypassed: " << (zone->isBypassed ? "Yes" : "No")
				<< std::endl;
			return;
		}
	}
	std::cout << "Zone with ID " << zoneId << " not found." << std::endl;
}
void AlarmService::ListArmedZones()
{
	std::cout << "Listing armed zones:" << std::endl;
	for (const auto& zone : zones)
	{
		if (zone->isArmed)
		{
			std::cout << "ID: " << zone->id
				<< ", Name: " << zone->name
				<< ", Type: " << zone->GetType()
				<< std::endl;
		}
	}
	if (zones.empty())
	{
		std::cout << "No zones available." << std::endl;
	}
}
void AlarmService::ListBypassedZones()
{
	std::cout << "Listing bypassed zones:" << std::endl;
	for (const auto& zone : zones)
	{
		if (zone->isBypassed)
		{
			std::cout << "ID: " << zone->id
				<< ", Name: " << zone->name
				<< ", Type: " << zone->GetType()
				<< std::endl;
		}
	}
	if (zones.empty())
	{
		std::cout << "No zones available." << std::endl;
	}
}
void AlarmService::ListDisarmedZones()
{
	std::cout << "Listing disarmed zones:" << std::endl;
	for (const auto& zone : zones)
	{
		if (!zone->isArmed)
		{
			std::cout << "ID: " << zone->id
				<< ", Name: " << zone->name
				<< ", Type: " << zone->GetType()
				<< std::endl;
		}
	}
	if (zones.empty())
	{
		std::cout << "No zones available." << std::endl;
	}
}
void AlarmService::ListAlarmingZones()
{
	std::cout << "Listing alarming zones:" << std::endl;
	for (const auto& zone : zones)
	{
		if (zone->isAlarming)
		{
			std::cout << "ID: " << zone->id
				<< ", Name: " << zone->name
				<< ", Type: " << zone->GetType()
				<< std::endl;
		}
	}
	if (zones.empty())
	{
		std::cout << "No zones available." << std::endl;
	}
}
std::shared_ptr<Zone> AlarmService::GetZoneById(int zoneId)
{
	for (const auto& zone : zones)
	{
		if (zone->id == zoneId)
		{
			return zone;
		}
	}
	return nullptr;
}
std::string AlarmService::GetZoneStatus(int zoneId)
{
	auto zone = GetZoneById(zoneId);
	if (!zone)
	{
		return "NOT_FOUND";
	}
	if (zone->isBypassed) {
		return "BYPASSED";
	}
	if (zone->isAlarming) {
		return "ALARMING";
	}
	if (zone->isArmed) {
		return "ARMED";
	}
	return "DISARMED";
}
void AlarmService::TriggerZone(int zoneId)
{
	auto zone = GetZoneById(zoneId);
	if (!zone)
	{
		std::cout << "Zone with ID " << zoneId << " not found for triggering." << std::endl;
		return;
	}
	if (zone->isBypassed) {
		std::cout << "Zone " << zoneId << " triggered but is BYPASSED. Ignoring." << std::endl;
		return;
	}
	if (zone->isArmed)
	{
		zone->isAlarming = true;
		std::cout << "ALARM TRIGGERED! Zone " << zoneId << " (" << zone->name << ") detected breach!" << std::endl;
	}
	else
	{
		std::cout << "Zone " << zoneId << " triggered but is DISARMED. Ignoring." << std::endl;
	}
}
void AlarmService::SaveState() {
	std::ofstream file("zone_state.txt");
	if (!file.is_open()) {
		std::cerr << "[ERROR] Could not save state to zone_state.txt" << std::endl;
		return;
	}
	for (const auto& zone : zones) {
		file << zone->id << ";"
			<< (zone->isArmed ? "1" : "0") << ";"
			<< (zone->isBypassed ? "1" : "0") << "\n";
	}
	file.close();
	std::cout << "[INFO] Zone states saved successfully" << std::endl;
}
void AlarmService::LoadState() {
	std::ifstream file("zone_state.txt");
	if (!file.is_open()) {
		std::cout << "[INFO] No saved state found (zone_state.txt)" << std::endl;
		return;
	}
	std::string line;
	while (std::getline(file, line)) {

		if (line.empty()) continue;

		std::stringstream ss(line);
		std::string segment;
		std::vector<std::string> parts;
		while (std::getline(ss, segment, ';')) {
			parts.push_back(segment);
		}
		if (parts.size() >= 3) {
			try
			{
				int zoneId = std::stoi(parts[0]);
				bool isArmed = (parts[1] == "1");
				bool isBypassed = (parts[2] == "1");
				auto zone = GetZoneById(zoneId);
				if (zone) {
					zone->isArmed = isArmed;
					zone->isBypassed = isBypassed;
				}
			}
			catch (const std::exception&)
			{
				std::cerr << "[ERROR] Corrupt data in state file." << std::endl;
			}
		}
	}
	file.close();
	std::cout << "[SYSTEM] Previous zone states loaded." << std::endl;
}

