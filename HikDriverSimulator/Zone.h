#pragma once
#include <string>
#include <iostream>
class Zone
{
public:
	int id;
	std::string name;
	bool isArmed;
	bool isAlarming;
	bool isBypassed;

	Zone(int zoneId, const std::string& zoneName);
	virtual ~Zone();
	void Arm();
	void Disarm();
	void SetBypass(bool active);
	virtual const std::string GetType();
};