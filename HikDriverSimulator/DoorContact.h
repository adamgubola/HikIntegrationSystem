#pragma once
#include "Zone.h"
class DoorContact : public Zone
{
	public:
	DoorContact(int zoneId, const std::string& zoneName)
		: Zone(zoneId, zoneName)
	{
	}
	const std::string GetType() override { return "Door Contact"; }
};