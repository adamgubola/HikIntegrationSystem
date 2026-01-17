# HikIntegrationSystem

This project is a C++ console application designed to simulate an alarm system driver. Its primary goal is to establish the core architecture and logic required for a future integration with HikCentral.

## Architecture

The project follows a **Layered Architecture** to ensure separation of concerns:

* **Models:** Represents hardware entities (`Zone`, `MotionSensor`, `DoorContact`) using OOP principles.
* **Service:** Handles the business logic and state management (`AlarmService`).
* **Network:** TCP Server implementation for external communication (WinSock2).
* **Data:** Loads initial configuration from a CSV file.

## Current Status

- [x] **Object-Oriented Design:** Polymorphic handling of different sensor types.
- [x] **Configuration:** Zones are initialized from `zones.csv` at startup.
- [x] **In-Memory Storage:** Real-time state management using `std::vector` and smart pointers.
- [x] **Console Interface:** Basic commands to Arm, Disarm, and Bypass zones.
- [ ] **Network Layer:** TCP Server integration is currently in progress.

## Tech Stack

* **Language:** C++
* **IDE:** Visual Studio 2022
* **Platform:** Windows (Win32 Console / WinSock2)

---
*Created by Adam Gubola.*