#pragma once
#include "Zone.h"
class DoorContact : public Zone
{
	public:
	DoorContact(int zoneId, const std::string& zoneName, int newPartitionId)
		: Zone(zoneId, zoneName, newPartitionId)
	{
	}
	const std::string GetType() override { return "Door Contact"; }
};