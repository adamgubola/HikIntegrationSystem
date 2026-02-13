#pragma once
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>
#include "Zone.h"
#include "Partition.h"



class AlarmService
{
private:
	std::vector<std::shared_ptr<Zone>> zones;
	std::vector<std::shared_ptr<Partition>> partitions;
	std::string CreateResponse(const std::string& status, const std::string& message, int id = -1, const std::string& state = "");
	nlohmann::json CreateZoneJson(const std::shared_ptr<Zone>& zone);

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
	std::shared_ptr<Partition> GetPartitionById(int partitionId);
	std::string ArmZone(int zoneId);
	std::string DisarmZone(int zoneId);
	std::string BypassZone(int zoneId, bool active);
	std::string TriggerZone(int zoneId);
	std::string ArmPartition(int partitionId);
	std::string DisarmPartition(int partitionId);
	void SaveStateToTxt();
	void LoadStateFromTxt();
	void SaveStateToJson();
	void LoadStateFromJson();

	};
