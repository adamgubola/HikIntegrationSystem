#pragma once
#include <vector>
#include <memory>
#include "Zone.h"

class AlarmService
{
private:
	std::vector<std::shared_ptr<Zone>> zones;
	std::string CreateResponse(const std::string& status, const std::string& message, int id = -1, const std::string& state = "");
	void WriteResponseToConsole();


public:
	AlarmService();
	~AlarmService();
	void InitializeZones();
	std::string ListAllZones();
	std::string ListOneZone(int zoneId);
	std::string ListArmedZones();
	std::string ListBypassedZones();
	std::string ListDisarmedZones();
	std::string ListAlarmingZones();
	std::string GetZoneStatus(int zoneId);
	std::shared_ptr<Zone> GetZoneById(int zoneId);
	std::string ArmZone(int zoneId);
	std::string DisarmZone(int zoneId);
	std::string BypassZone(int zoneId, bool active);
	std::string TriggerZone(int zoneId);
	void SaveStateToTxt();
	void LoadStateFromTxt();
	void SaveStateToJson();
	void LoadStateFromJson();

	};
