#include "AlarmService.h"
#include "MotionSenzor.h"
#include "DoorContact.h"
#include "Zone.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>
#include "Logger.h"	

AlarmService::AlarmService() {}
AlarmService::~AlarmService() {}
using json = nlohmann::json;

// Initialize zones from zones.csv
void AlarmService::InitializeZones()
{
	Logger::Info("Initializing zones from zones.csv");
	zones.clear();
	std::ifstream file("zones.csv");
	if (!file.is_open())
	{
		Logger::Error("Failed to open zones.csv");
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
			Logger::Info("Added zone: ID=" + std::to_string(zoneId) + ", Name=" + zoneName + ", Type=" + zoneType);
		}
		else {
			Logger::Error("Invalid line in zones.csv: " + line);
		}

	}
	file.close();
	Logger::Info(std::to_string(zones.size()) + " zones initialized.");

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
	Logger::Warning("Zone with ID " + std::to_string(zoneId) + " not found.");

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
	Logger::Warning("Zone with ID " + std::to_string(zoneId) + " not found.");
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
	Logger::Warning("Zone with ID " + std::to_string(zoneId) + " not found.");
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
		Logger::Info("No zones available to list.");
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
	Logger::Warning("Zone with ID " + std::to_string(zoneId) + " not found.");
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
		Logger::Info("No zones available to list.");
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
		Logger::Info("No zones available to list.");
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
		Logger::Info("No zones available to list.");
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
		Logger::Info("No zones available to list.");
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
		Logger::Info("Zone with ID " + std::to_string(zoneId) + " not found for triggering.");
		return;
	}
	if (zone->isBypassed) {
		Logger::Info("Zone " + std::to_string(zoneId) + " is bypassed. Ignoring trigger.");
		return;
	}
	if (zone->isArmed)
	{
		zone->isAlarming = true;
		Logger::Warning("ALARM TRIGGERED! Zone " + std::to_string(zoneId) + " (" + zone->name + ") detected breach!");
	}
	else
	{
		Logger::Info("Zone " + std::to_string(zoneId) + " triggered but is DISARMED. Ignoring.");
	}
}
void AlarmService::SaveStateToTxt() {
	std::ofstream file("zone_state.txt");
	if (!file.is_open()) {
		Logger::Error("Could not save state to zone_state.txt");
		return;
	}
	for (const auto& zone : zones) {
		file << zone->id << ";"
			<< (zone->isArmed ? "1" : "0") << ";"
			<< (zone->isBypassed ? "1" : "0") << "\n";
	}
	file.close();
	Logger::Info("Zone states saved successfully to zone_state.txt");
}
void AlarmService::LoadStateFromTxt() {
	std::ifstream file("zone_state.txt");
	if (!file.is_open()) {
		Logger::Warning("No saved state found (zone_state.txt)");
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
				Logger::Error("Corrupt data in state file.");
			}
		}
	}
	file.close();
	Logger::Info("Previous zone states loaded from zone_state.txt");
}
void AlarmService::SaveStateToJson() {

	json JArray;
	for (const auto& zone : zones) {

		json jZone;
		jZone["id"] = zone->id;
		jZone["armed"] = zone->isArmed;
		jZone["bypassed"] = zone->isBypassed;


		JArray.push_back(jZone);
	}
	std::ofstream file("zone_state.json");
	if (file.is_open()) {
		file << JArray.dump(4);
		file.close();
		Logger::Info("Zone states saved successfully to JSON.");
	}
	else {
		Logger::Error("Could not save state to zone_state.json");
	}
}
void AlarmService::LoadStateFromJson() {
	std::ifstream file("zone_state.json");
	if (!file.is_open()) {
		Logger::Warning("No saved JSON state found (zone_state.json)");
		return;
	}
	json JArray;
	try
	{
		file >> JArray;

		for (const auto& jZone : JArray)
		{
			if (jZone.contains("id") && jZone.contains("armed") && jZone.contains("bypassed"))
			{
				int zoneId = jZone["id"];
				bool isArmed = jZone["armed"];
				bool isBypassed = jZone["bypassed"];
				auto zone = GetZoneById(zoneId);
				if (zone) {
					zone->isArmed = isArmed;
					zone->isBypassed = isBypassed;
				}
			}
		}
		Logger::Info("Previous zone states loaded from JSON.");
	}
	catch (const json::parse_error& e)
	{
		Logger::Error("JSON parse error while loading state: " + std::string(e.what()));
	}
	catch (const std::exception& e)
	{
		Logger::Error("Exception while loading JSON state: " + std::string(e.what()));
	}
	file.close();
};



