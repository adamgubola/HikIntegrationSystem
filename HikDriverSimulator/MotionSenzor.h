#pragma once
#include "Zone.h"
class MotionSenzor : public Zone
{
public:
	MotionSenzor(int zoneId, const std::string& zoneName, int newPartitionId)
		: Zone(zoneId, zoneName, newPartitionId)
	{
	}
	const std::string GetType() override { return "Motion Sensor"; }

};