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
	// Temporary partitions for test
	partitions.clear();
	partitions.push_back(std::make_shared<Partition>(1, "Default Partition"));
	partitions.push_back(std::make_shared<Partition>(2, "Garage Partition"));
	Logger::Info("Initialized 2 dummy partitions.");

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
			int zonePartitionId = -1;

			if (parts.size() >= 4) 
			{
				try
				{
					zonePartitionId = std::stoi(parts[3]);

				}
				catch (const std::exception&)
				{
					Logger::Error("Error while reading partition data for zone " + std::to_string(zoneId));
				}
			}

			std::shared_ptr<Zone> newZone;
			if (zoneType == "Motion Sensor")
			{
				newZone = std::make_shared<MotionSenzor>(zoneId, zoneName, zonePartitionId);
			}
			else if (zoneType == "Door Contact")
			{
				newZone = std::make_shared<DoorContact>(zoneId, zoneName, zonePartitionId);
			}
			else
			{
				newZone = std::make_shared<Zone>(zoneId, zoneName, zonePartitionId);
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
		return CreateResponse("IGNORED", "Zone is already armed", zoneId, "ARMED");
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
		return CreateResponse("IGNORED", "Zone is already disarmed", zoneId, "DISARMED");
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
		json jZone = CreateZoneJson(zone);
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
		jZone = CreateZoneJson(zone);
		jZone["status"] = "SUCCESS";

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
			json jZone = CreateZoneJson(zone);
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
			json jZone = CreateZoneJson(zone);
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
			json jZone = CreateZoneJson(zone);
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
			json jZone = CreateZoneJson(zone);
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
std::shared_ptr<Partition> AlarmService::GetPartitionById(int partitionId)
{
	for (const auto& part : partitions)
	{
		if (part->id == partitionId)
		{
			return part;
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
	json jSystem;

	json jPartitions = json::array();
	for (const auto& partition : partitions) {
		json jPart;
		jPart["id"] = partition->id;
		jPart["name"] = partition->name;
		jPart["armed"] = partition->isArmed;
		jPartitions.push_back(jPart);
	}
	jSystem["partitions"] = jPartitions;

	json jZones = json::array();
	for (const auto& zone : zones) {
		jZones.push_back(CreateZoneJson(zone));
	}
	jSystem["zones"] = jZones;

	std::ofstream file("system_state.json");
	if (file.is_open()) {
		file << jSystem.dump(4);
		file.close();
		Logger::Info("System state (Partitions and Zones) saved successfully to JSON.");
	}
	else {
		Logger::Error("Could not save state to system_state.json");
	}
}
void AlarmService::LoadStateFromJson() {
	std::ifstream file("system_state.json");
	if (!file.is_open()) {
		Logger::Warning("No saved JSON state found (system_state.json). Using default states.");
		return;
	}

	json jSystem;
	try {
		file >> jSystem;

		if (jSystem.contains("partitions") && jSystem["partitions"].is_array()) {
			for (const auto& jPart : jSystem["partitions"]) {
				if (jPart.contains("id")) {
					int pId = jPart["id"];
					auto partition = GetPartitionById(pId);

					if (partition) {
						if (jPart.contains("armed")) partition->isArmed = jPart["armed"];
						if (jPart.contains("name")) partition->name = jPart["name"];
					}
					else {
						std::string pName = jPart.value("name", "Unknown Partition");
						auto newPart = std::make_shared<Partition>(pId, pName);
						if (jPart.contains("armed")) newPart->isArmed = jPart["armed"];
						partitions.push_back(newPart);
					}
				}
			}
			Logger::Info("Partition data loaded from JSON backup.");
		}

		if (jSystem.contains("zones") && jSystem["zones"].is_array()) {
			for (const auto& jZone : jSystem["zones"]) {
				if (jZone.contains("id")) {
					int zoneId = jZone["id"];
					auto zone = GetZoneById(zoneId);

					if (zone) {
						if (jZone.contains("name")) zone->name = jZone["name"];
						if (jZone.contains("partitionId")) zone->partitionId = jZone["partitionId"];

						if (jZone.contains("armed")) zone->isArmed = jZone["armed"];
						if (jZone.contains("bypassed")) zone->isBypassed = jZone["bypassed"];
						if (jZone.contains("active")) zone->isActive = jZone["active"];
						if (jZone.contains("tampered")) zone->isTampered = jZone["tampered"];
						if (jZone.contains("faulted")) zone->isFaulted = jZone["faulted"];

						if (jZone.contains("alarming") && zone->isArmed && !zone->isBypassed) {
							zone->isAlarming = jZone["alarming"];
						}
					}
					else {
						//Future function for full backup
						Logger::Warning("Zone " + std::to_string(zoneId) + " found in backup but not in CSV. Skipping.");
					}
				}
			}
			Logger::Info("Zone data loaded from JSON backup.");
		}
	}
	catch (const json::parse_error& e) {
		Logger::Error("JSON parse error while loading state: " + std::string(e.what()));
	}
	catch (const std::exception& e) {
		Logger::Error("Exception while loading JSON state: " + std::string(e.what()));
	}
	file.close();
}std::string AlarmService::CreateResponse(const std::string& status, const std::string& message, int id, const std::string& state) {
	json responseJson;
	responseJson["status"] = status;
	responseJson["message"] = message;
	if (id != -1) responseJson["id"] = id;
	if (!state.empty()) responseJson["newState"] = state;
	return responseJson.dump();

}
std::string AlarmService::ArmPartition(int partitionId)
{
	Logger::Info("Request to ARM Partition: " + std::to_string(partitionId));

	auto partition = GetPartitionById(partitionId);
	if (!partition)
	{
		Logger::Warning("ArmPartition failed: Partition " + std::to_string(partitionId) + " does not exist.");
		return CreateResponse("ERROR", "Partition not found", partitionId);
	}
	if (partition->isArmed) 
	{
		Logger::Info("Partition " + std::to_string(partitionId) + " already armed.");
		return CreateResponse("IGNORED", "Partition already armed", partitionId, "ARMED");
	}

	json errorList = json::array();
	std::vector<std::shared_ptr<Zone>> zonesToArm;

	for (auto& zone : zones) {
		if (zone->partitionId == partitionId) {

			if (zone->isBypassed) {
				zonesToArm.push_back(zone);
				Logger::Info("Zone " + std::to_string(zone->id) + " is bypassed. Ignoring status checks.");
				continue; 
			}
			if (zone->isTampered) {
				json errorItem;
				errorItem["id"] = zone->id;
				errorItem["name"] = zone->name;
				errorItem["bypassed"] = zone->isBypassed;
				errorItem["reason"] = "ZONE_TAMPERED";
				errorList.push_back(errorItem);
			}
			else if (zone->isFaulted) {
				json errorItem;
				errorItem["id"] = zone->id;
				errorItem["name"] = zone->name;
				errorItem["bypassed"] = zone->isBypassed;
				errorItem["reason"] = "ZONE_FAULTED";
				errorList.push_back(errorItem);
			}
			else if (zone->isActive) {
				json errorItem;
				errorItem["id"] = zone->id;
				errorItem["name"] = zone->name;
				errorItem["bypassed"] = zone->isBypassed;
				errorItem["reason"] = "ZONE_ACTIVE";
				errorList.push_back(errorItem);
			}
			else {
				zonesToArm.push_back(zone);
			}
		}
	}
	if (!errorList.empty()) {
		json response;
		response["status"] = "ERROR";
		response["message"] = "Partition not ready";
		response["partitionId"] = partitionId;
		response["faultedZones"] = errorList;

		Logger::Warning("Arming Partition " + std::to_string(partitionId) + " failed due to active/faulted zones.");
		return response.dump();
	}
	int armedCount = 0;
	for (auto& zone : zonesToArm) {

		if (!zone->isArmed) {
			zone->Arm();

			if (zone->isArmed) {
				armedCount++;
			}
		}
	}
	partition->isArmed = true;
	return CreateResponse("SUCCESS", "Partition with "+ std::to_string(armedCount) + " zones, armed successfully", partitionId, "ARMED");
}
std::string AlarmService::DisarmPartition(int partitionId)
{
	Logger::Info("Request to DISARM Partition: " + std::to_string(partitionId));

	auto partition = GetPartitionById(partitionId);
	if (!partition) {
		return CreateResponse("ERROR", "Partition not found", partitionId);
	}
	if (!partition->isArmed)
	{
		Logger::Info("Partition " + std::to_string(partitionId) + " already armed.");
		return CreateResponse("IGNORED", "Partition already disarmed", partitionId, "DISARMED");
	}

	for (auto& zone : zones) {
		if (zone->partitionId == partitionId) {
			zone->Disarm();
		}
	}
	partition->isArmed = false;
	return CreateResponse("SUCCESS", "Partition disarmed", partitionId, "DISARMED");
}
json AlarmService::CreateZoneJson(const std::shared_ptr<Zone>& zone)
{
	json jZone; 
	jZone["id"] = zone->id; 
	jZone["name"] = zone->name; 
	jZone["type"] = zone->GetType(); 
	jZone["armed"] = zone->isArmed; 
	jZone["bypassed"] = zone->isBypassed; 
	jZone["alarming"] = zone->isAlarming; 
	jZone["active"] = zone->isActive;
	jZone["tampered"] = zone->isTampered;
	jZone["faulted"] = zone->isFaulted;
	jZone["partitionId"] = zone->partitionId;
	return jZone;
}
