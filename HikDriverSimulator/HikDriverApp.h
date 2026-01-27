#pragma once
#include <memory>
#include <iostream>
#include <string>

#include "AlarmService.h"
#include "TcpServer.h"

class HikDriverApp {
private:
	AlarmService alarmService;
	std::unique_ptr<TcpServer> tcpServer;
	bool isRunning;
	void ShowMenu();

public:
	HikDriverApp();
	~HikDriverApp();
	void Run();

};