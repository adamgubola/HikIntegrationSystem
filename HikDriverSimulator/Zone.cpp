#include "Zone.h"
#include <iostream>

Zone::Zone(int zoneId, const std::string& zoneName)
	: id(zoneId), name(zoneName), isArmed(false), isAlarming(false), isBypassed(false)
{
}
Zone::~Zone() {}

void Zone::Arm()
{
	if(isArmed)
	{
		std::cout << "Zone " << id << " (" << name << ") is already armed." << std::endl;
		return;
	}
	if (!isBypassed)
	{
		isArmed = true;
		isAlarming = false;
		std::cout << "Zone " << id << " (" << name << ") is armed." << std::endl;
	}
	else
	{
		std::cout << "Zone " << id << " (" << name << ") cannot be armed because it is bypassed." << std::endl;
	}
}
void Zone::Disarm()
{
	if(!isArmed)
	{
		std::cout << "Zone " << id << " (" << name << ") is already disarmed." << std::endl;
		return;
	}
	isArmed = false;
	isAlarming = false;
	std::cout << "Zone " << id << " (" << name << ") is disarmed." << std::endl;
}
void Zone::SetBypass(bool active)
{
	isBypassed = active;
	if(active)
	{
		std::cout << "Zone " << id << " (" << name << ") is bypassed." << std::endl;
	}
	else
	{
		std::cout << "Zone " << id << " (" << name << ") is unbypassed." << std::endl;
	}

}
const std::string Zone::GetType() { return "Generic Zone"; }


