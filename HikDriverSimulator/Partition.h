#pragma once
#include <string>

class Partition 
{
public:
	int id;
	std::string name;
	bool isArmed;
	Partition(int partitionId, const std::string partitionName);
	~Partition();



};
