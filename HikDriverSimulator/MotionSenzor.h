#pragma once
#include "Zone.h"
class MotionSenzor : public Zone
{
public:
	MotionSenzor(int zoneId, const std::string& zoneName)
		: Zone(zoneId, zoneName)
	{
	}
	const std::string GetType() override { return "Motion Sensor"; }

};