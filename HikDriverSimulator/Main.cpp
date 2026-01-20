#include <iostream>
#include <limits>
#include "AlarmService.h"
#define NOMINMAX
#include "TcpServer.h"


void ShowMenu() {
    std::cout << "\n================ HIKVISION DRIVER SIMULATOR ================" << std::endl;
    std::cout << "--- Controls ---" << std::endl;
    std::cout << "1. Arm Zone" << std::endl;
    std::cout << "2. Disarm Zone" << std::endl;
    std::cout << "3. Bypass Zone" << std::endl;
    std::cout << "4. Unbypass Zone" << std::endl;

    std::cout << "\n--- Monitoring / Lists ---" << std::endl;
    std::cout << "5. List All Zones" << std::endl;
    std::cout << "6. List ARMED Zones Only" << std::endl;
    std::cout << "7. List DISARMED Zones Only" << std::endl;
    std::cout << "8. List BYPASSED Zones Only" << std::endl;
    std::cout << "9. List ALARMING Zones Only" << std::endl;
    std::cout << "10. Find Zone by ID" << std::endl;

    std::cout << "0. Exit" << std::endl;
    std::cout << "Select option: ";
}

int main() {
    AlarmService service;
    service.InitializeZones();

    TcpServer server(12345, &service);
    if (!server.Start()) {
        std::cerr << "CRITICAL ERROR : Could not start TCP Server!" << std::endl;
        return -1;
    }

    int choice = -1;
    int id = 0;

    while (choice != 0) {
        ShowMenu();

        if (!(std::cin >> choice)) {
            std::cout << "ERROR: Please enter a valid number!" << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        std::cout << "\n";

        switch (choice) {
        case 1: // Arm
            std::cout << "Enter Zone ID to ARM: ";
            std::cin >> id;
            service.ArmZone(id);
            break;
        case 2: // Disarm
            std::cout << "Enter Zone ID to DISARM: ";
            std::cin >> id;
            service.DisarmZone(id);
            break;
        case 3: // Bypass
            std::cout << "Enter Zone ID to BYPASS: ";
            std::cin >> id;
            service.BypassZone(id, true);
            break;
        case 4: // Unbypass
            std::cout << "Enter Zone ID to UNBYPASS: ";
            std::cin >> id;
            service.BypassZone(id, false);
            break;
        case 5:
            service.ListAllZones();
            break;
        case 6:
            service.ListArmedZones();
            break;
        case 7:
            service.ListDisarmedZones();
            break;
        case 8:
            service.ListBypassedZones();
            break;
        case 9:
            service.ListAlarmingZones();
            break;
        case 10:
            std::cout << "Enter Zone ID to find: ";
            std::cin >> id;
            service.ListOneZone(id);
            break;
        case 0:
            std::cout << "Exiting system..." << std::endl;
            break;
        default:
            std::cout << "Unknown command. Please try again." << std::endl;
        }

        // Wait for user input before clearing or showing menu again
        if (choice != 0) {
            std::cout << "\n(Press Enter to continue...)";
            std::cin.ignore();
            std::cin.get();
        }
    }
    server.Stop();

    return 0;
}