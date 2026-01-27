#pragma once
#include <vector>
#include <memory>
#include "Zone.h"

class AlarmService
{
private:
	std::vector<std::shared_ptr<Zone>> zones;

public:
	AlarmService();
	~AlarmService();
	void InitializeZones();
	void ListAllZones();
	void ListOneZone(int zoneId);
	void ListArmedZones();
	void ListBypassedZones();
	void ListDisarmedZones();
	void ListAlarmingZones();
	std::shared_ptr<Zone> GetZoneById(int zoneId);
	void ArmZone(int zoneId);
	void DisarmZone(int zoneId);
	void BypassZone(int zoneId, bool active);
	std::string GetZoneStatus(int zoneId);
	void TriggerZone(int zoneId);
	void SaveStateToTxt();
	void LoadStateFromTxt();
	void SaveStateToJson();
	void LoadStateFromJson();
	};
