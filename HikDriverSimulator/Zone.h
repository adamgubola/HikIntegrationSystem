#pragma once
#include <string>
#include <iostream>
class Zone
{
public:
	int id;
	int partitionId;
	std::string name;
	bool isArmed;
	bool isAlarming;
	bool isBypassed;
	bool isActive;
	bool isTampered;
	bool isFaulted;

	Zone(int zoneId, const std::string& zoneName, int newPartitionId);
	virtual ~Zone();
	void Arm();
	void Disarm();
	void SetBypass(bool active);
	virtual const std::string GetType();
	void SetPartitionId(int nemPartitionId);
	void SetTampered(bool tampered);
	void SetFaulted(bool faulted);
	void SetActive(bool active);
};