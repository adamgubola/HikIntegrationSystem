#include "Zone.h"
#include <iostream>
#include "Logger.h"

Zone::Zone(int zoneId, const std::string& zoneName)
	: id(zoneId), name(zoneName), isArmed(false), isAlarming(false), isBypassed(false)
{
}
Zone::~Zone() {}

void Zone::Arm()
{
	if(isArmed)
	{
		Logger::Info("Zone " + std::to_string(id) + " (" + name + ") is already armed.");
		return;
	}
	if (!isBypassed)
	{
		isArmed = true;
		isAlarming = false;
		Logger::Info("Zone " + std::to_string(id) + " (" + name + ") is armed.");
	}
	else
	{
		Logger::Info("Zone " + std::to_string(id) + " (" + name + ") cannot be armed because it is bypassed.");
	}
}
void Zone::Disarm()
{
	if(!isArmed)
	{
		Logger::Info("Zone " + std::to_string(id) + " (" + name + ") is already disarmed.");
		return;
	}
	isArmed = false;
	isAlarming = false;
	Logger::Info("Zone " + std::to_string(id) + " (" + name + ") is disarmed.");
}
void Zone::SetBypass(bool active)
{
	isBypassed = active;
	if(active)
	{
		Logger::Info("Zone " + std::to_string(id) + " (" + name + ") is bypassed.");
	}
	else
	{
		Logger::Info("Zone " + std::to_string(id) + " (" + name + ") is unbypassed.");
	}

}
const std::string Zone::GetType() { return "Generic Zone"; }


