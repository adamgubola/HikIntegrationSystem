#include "Zone.h"
#include <iostream>
#include "Logger.h"

Zone::Zone(int zoneId, const std::string& zoneName, int newPartitionId = -1)
	: id(zoneId),
	name(zoneName),
	partitionId(newPartitionId),
	isArmed(false),
	isAlarming(false),
	isBypassed(false),
	isActive(false),  
	isTampered(false), 
	isFaulted(false)
	
{
}
Zone::~Zone() {}

void Zone::Arm()
{
	if (isArmed)
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
	if (!isArmed)
	{
		Logger::Info("Zone " + std::to_string(id) + " (" + name + ") is already disarmed.");
		return;
	}
	isArmed = false;
	isAlarming = false;
	Logger::Info("Zone " + std::to_string(id) + " (" + name + ") is disarmed.");
}
void Zone::SetBypass(bool bypassState)
{
	isBypassed = bypassState;
	if (bypassState)
	{
		Logger::Info("Zone " + std::to_string(id) + " (" + name + ") is bypassed.");
	}
	else
	{
		Logger::Info("Zone " + std::to_string(id) + " (" + name + ") is unbypassed.");
	}

}
const std::string Zone::GetType() { return "Generic Zone"; }

void Zone::SetPartitionId(int newPartitionId) {	this->partitionId = newPartitionId;}
void Zone::SetTampered(bool tampered) 
{ 
	isTampered = tampered;
	if (tampered) 
	{ 
		Logger::Warning("Zone " + std::to_string(id) + " (" + name + ") is tampered!"); 
		if (isArmed) {
			isAlarming = true;
			Logger::Warning("ALARM on Zone " + std::to_string(id) + " (" + name + ")!");
		}
	}
	else 
	{ 
		Logger::Info("Zone " + std::to_string(id) + " (" + name + ") tamper cleared."); 
	}
}
void Zone::SetFaulted(bool faulted) 
{ 
	this->isFaulted = faulted;
	if (faulted) 
	{ 
		Logger::Warning("Zone " + std::to_string(id) + " (" + name + ") is faulted!"); 
		if (isArmed) {
			isAlarming = true;
			Logger::Warning("ALARM on Zone " + std::to_string(id) + " (" + name + ")!");
		}
	}
	else
	{ 
		Logger::Info("Zone " + std::to_string(id) + " (" + name + ") fault cleared."); 
	}
}
void Zone::SetActive(bool active) 
{ 
	this->isActive = active; 
	if (active) 
	{ 
		Logger::Info("Zone " + std::to_string(id) + " (" + name + ") is active."); 
		if (isArmed) {
			isAlarming = true;
			Logger::Warning("ALARM on Zone " + std::to_string(id) + " (" + name + ")!");
		}
	}
	else 
	{ 
		Logger::Info("Zone " + std::to_string(id) + " (" + name + ") is inactive."); 
	}

}




