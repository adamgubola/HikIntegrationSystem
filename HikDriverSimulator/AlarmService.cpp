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
std::string AlarmService::ArmZone(int zoneId)
{
	auto zone = GetZoneById(zoneId);

	if (!zone) {
		Logger::Warning("Arm failed: Zone " + std::to_string(zoneId) + " not found.");
		return CreateResponse("ERROR", "Zone not found", zoneId);
	}
	if (zone->isArmed) {
		Logger::Info("Arm request: Zone " + std::to_string(zoneId) + " already armed.");
		return CreateResponse("INFO", "Zone is already armed", zoneId, "ARMED");
	}
	if (zone->isBypassed) {
		Logger::Info("Arm failed: Zone " + std::to_string(zoneId) + " is bypassed.");
		return CreateResponse("INFO", "Zone is bypassed, failed to arm", zoneId, "BYPASSED");
	}

	zone->Arm();
	return CreateResponse("SUCCESS", "Zone " + std::to_string(zoneId) + " armed successfully", zoneId, "ARMED");
}
std::string AlarmService::DisarmZone(int zoneId) {
	auto zone = GetZoneById(zoneId);

	if (!zone) {
		Logger::Info("Disarm failed: Zone " + std::to_string(zoneId) + " not found.");
		return CreateResponse("ERROR", "Zone not found", zoneId);
	}
	if (!zone->isArmed) {
		Logger::Info("Disarm request: Zone " + std::to_string(zoneId) + " already disarmed.");
		return CreateResponse("INFO", "Zone is already disarmed", zoneId, "DISARMED");
	}

	zone->Disarm();
	return CreateResponse("SUCCESS", "Zone " + std::to_string(zoneId) + " disarmed successfully", zoneId, "DISARMED");
}
std::string AlarmService::BypassZone(int zoneId, bool active) {
	auto zone = GetZoneById(zoneId);
	if (!zone) {
		Logger::Info("Bypass failed: Zone " + std::to_string(zoneId) + " not found.");
		return CreateResponse("ERROR", "Zone not found", zoneId);
	}
	zone->SetBypass(active);
	std::string state = active ? "BYPASSED" : "UNBYPASSED";
	std::string msg = active ? "bypassed" : "unbypassed";

	return CreateResponse("SUCCESS", "Zone " + std::to_string(zoneId) + " " + msg, zoneId, state);
}
std::string AlarmService::GetZoneStatus(int zoneId)
{
	auto zone = GetZoneById(zoneId);
	if (!zone) return CreateResponse("ERROR", "Zone not found", zoneId);

	std::string statusStr;
	if (zone->isBypassed) statusStr = "BYPASSED";
	else if (zone->isAlarming) statusStr = "ALARMING";
	else if (zone->isArmed) statusStr = "ARMED";
	else statusStr = "DISARMED";

	return CreateResponse("SUCCESS", "Status query", zoneId, statusStr);
}
std::string AlarmService::TriggerZone(int zoneId)
{
	auto zone = GetZoneById(zoneId);
	if (!zone)
	{
		Logger::Info("Trigger failed " + std::to_string(zoneId) + " not found.");
		return CreateResponse("ERROR", "Zone not found", zoneId);
	}
	if (zone->isBypassed) {
		Logger::Info("Trigger ignored: Zone " + std::to_string(zoneId) + " is bypassed.");
		return CreateResponse("IGNORED", "Zone is bypassed", zoneId, "BYPASSED");
	}
	if (zone->isArmed)
	{
		zone->isAlarming = true;
		Logger::Warning("ALARM TRIGGERED on Zone " + std::to_string(zoneId));
		return CreateResponse("ALARM", "Zone is triggered " + std::to_string(zoneId), zoneId, "ALARMING");
	}
	else {
		Logger::Info("Trigger ignored: Zone " + std::to_string(zoneId) + " is disarmed.");
		return CreateResponse("IGNORED", "Zone is disarmed", zoneId, "DISARMED");
	}
}
std::string AlarmService::ListAllZones()
{
	Logger::Info("Listing all zones: ");

	json jArray = json::array();

	for (const auto& zone : zones)
	{
		json jZone;
		jZone["id"] = zone->id;
		jZone["name"] = zone->name;
		jZone["type"] = zone->GetType();
		jZone["armed"] = zone->isArmed;
		jZone["bypassed"] = zone->isBypassed;
		jZone["alarming"] = zone->isAlarming;
		jArray.push_back(jZone);
	}
	if (zones.empty())
	{
		Logger::Info("No zones available to list.");
		return jArray.dump();
	}
	else {
		Logger::Info("All zones listed");
		return jArray.dump();
	}
}
std::string AlarmService::ListOneZone(int zoneId)
{
	Logger::Info("Listing chosen zone:");

	auto zone = GetZoneById(zoneId);
	json jZone;

	if (!zone) {
		Logger::Warning("ListOneZone: Zone " + std::to_string(zoneId) + " not found.");
		return CreateResponse("ERROR", "Zone not found", zoneId);
	}
	else {
		jZone["status"] = "SUCCESS";
		jZone["id"] = zone->id;
		jZone["name"] = zone->name;
		jZone["type"] = zone->GetType();
		jZone["armed"] = zone->isArmed;
		jZone["bypassed"] = zone->isBypassed;
		jZone["alarming"] = zone->isAlarming;

		Logger::Info("Chosen zone listed");
		return jZone.dump();
	}
}
std::string AlarmService::ListArmedZones()
{
	Logger::Info("Listing armed zones:");
	json jArray = json::array();

	for (const auto& zone : zones)
	{
		if (zone->isArmed)
		{
			json jZone;
			jZone["id"] = zone->id;
			jZone["name"] = zone->name;
			jZone["type"] = zone->GetType();
			jZone["armed"] = zone->isArmed;
			jZone["bypassed"] = zone->isBypassed;
			jZone["alarming"] = zone->isAlarming;
			jArray.push_back(jZone);
		}
	}
	if (zones.empty())
	{
		Logger::Info("No zones available to list.");
		return jArray.dump();
	}
	else {
		Logger::Info("All armed zones listed");
		return jArray.dump();
	}
}
std::string AlarmService::ListBypassedZones()
{
	Logger::Info("Listing bypassed zones:");

	json jArray = json::array();

	for (const auto& zone : zones)
	{
		if (zone->isBypassed)
		{
			json jZone;
			jZone["id"] = zone->id;
			jZone["name"] = zone->name;
			jZone["type"] = zone->GetType();
			jZone["armed"] = zone->isArmed;
			jZone["bypassed"] = zone->isBypassed;
			jZone["alarming"] = zone->isAlarming;
			jArray.push_back(jZone);
		}
	}
	if (zones.empty())
	{
		Logger::Info("No zones available to list.");
		return jArray.dump();
	}
	else {
		Logger::Info("All bypassed zones listed");
		return jArray.dump();
	}
}
std::string AlarmService::ListDisarmedZones()
{
	Logger::Info("Listing disarmed zones:");

	json jArray = json::array();

	for (const auto& zone : zones)
	{
		if (!zone->isArmed)
		{
			json jZone;
			jZone["id"] = zone->id;
			jZone["name"] = zone->name;
			jZone["type"] = zone->GetType();
			jZone["armed"] = zone->isArmed;
			jZone["bypassed"] = zone->isBypassed;
			jZone["alarming"] = zone->isAlarming;
			jArray.push_back(jZone);
		}
	}
	if (zones.empty())
	{
		Logger::Info("No zones available to list.");
		return jArray.dump();
	}
	else {
		Logger::Info("All disarmed zones listed");
		return jArray.dump();
	}
}
std::string AlarmService::ListAlarmingZones()
{
	Logger::Info("Listing alarming zones:");

	json jArray = json::array();

	for (const auto& zone : zones)
	{
		if (zone->isAlarming)
		{
			json jZone;
			jZone["id"] = zone->id;
			jZone["name"] = zone->name;
			jZone["type"] = zone->GetType();
			jZone["armed"] = zone->isArmed;
			jZone["bypassed"] = zone->isBypassed;
			jZone["alarming"] = zone->isAlarming;
			jArray.push_back(jZone);
		}
	}
	if (zones.empty())
	{
		Logger::Info("No zones available to list.");
		return jArray.dump();
	}
	else {
		Logger::Info("All alarming zones listed");
		return jArray.dump();
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
}
std::string AlarmService::CreateResponse(const std::string& status, const std::string& message, int id, const std::string& state) {
	json responseJson;
	responseJson["status"] = status;
	responseJson["message"] = message;
	if (id != -1) responseJson["id"] = id;
	if (!state.empty()) responseJson["newState"] = state;
	return responseJson.dump();

}




