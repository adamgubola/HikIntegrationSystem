#include <string>
#include <iostream>
#include "Logger.h"
#include "Partition.h"

Partition::Partition(int partitionId, const std::string partitionName)
	: id(partitionId), name(partitionName), isArmed(false)
{}
Partition::~Partition() {}


