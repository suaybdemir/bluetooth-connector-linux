/**
 * @file main.cpp
 * @brief ToothDroid - Modern Linux Bluetooth Manager
 * @version 2.0.0
 *
 * A command-line Bluetooth device manager for Linux systems.
 * Features: Device scanning, pairing, connecting, and audio profile management.
 */

#include <iostream>
#include <memory>
#include <signal.h>
#include <vector>

#include "include/BluetoothDevice.h"
#include "include/BluetoothManager.h"
#include "include/UI.h"

using namespace ToothDroid;

// Global manager instance for signal handling
std::unique_ptr<BluetoothManager> g_manager;

/**
 * @brief Signal handler for graceful shutdown
 */
void signalHandler(int signal) {
  (void)signal; // Suppress unused parameter warning
  std::cout << std::endl;
  UI::printWarning("Received interrupt signal. Cleaning up...");
  if (g_manager) {
    g_manager->disconnectDevice();
  }
  UI::printInfo("Goodbye!");
  exit(0);
}

/**
 * @brief Display the main menu
 */
void showMainMenu() {
  const std::string menuItems[] = {"🔍 Scan for devices",
                                   "📋 Show paired devices",
                                   "🔗 Connect to device",
                                   "🔌 Disconnect",
                                   "⭐ Quick connect (favorites)",
                                   "⚙️  Adapter settings",
                                   "🚪 Exit"};

  UI::printMenu(menuItems, 7);
}

/**
 * @brief Device selection submenu
 */
int deviceSelectionMenu(BluetoothManager &manager) {
  manager.displayDevices();

  auto &devices = manager.getDiscoveredDevices();
  if (devices.empty()) {
    return -1;
  }

  std::cout << "  " << UI::Color::DIM << "[0] Cancel" << UI::Color::RESET
            << std::endl;
  std::cout << std::endl;

  return UI::promptChoice("Select device:", 0, devices.size());
}

/**
 * @brief Device action submenu
 */
void deviceActionMenu(BluetoothManager &manager, BluetoothDevice &device) {
  UI::printDivider();
  std::cout << std::endl;
  UI::printInfo("Selected: " + device.getDisplayName());
  std::cout << "  MAC: " << UI::Color::DIM << device.macAddress
            << UI::Color::RESET << std::endl;
  std::cout << "  Status: ";
  if (device.isConnected) {
    std::cout << UI::Color::GREEN << "Connected" << UI::Color::RESET;
  } else if (device.isPaired) {
    std::cout << UI::Color::YELLOW << "Paired" << UI::Color::RESET;
  } else {
    std::cout << UI::Color::DIM << "Not paired" << UI::Color::RESET;
  }
  std::cout << std::endl;

  if (device.hasAudioSupport()) {
    std::cout << "  Audio: ";
    if (device.supportsA2DP)
      std::cout << UI::Color::CYAN << "A2DP " << UI::Color::RESET;
    if (device.supportsHSP)
      std::cout << UI::Color::CYAN << "HSP " << UI::Color::RESET;
    if (device.supportsHFP)
      std::cout << UI::Color::CYAN << "HFP " << UI::Color::RESET;
    std::cout << std::endl;
  }
  std::cout << std::endl;

  const std::string actions[] = {device.isConnected ? "Disconnect" : "Connect",
                                 device.isPaired ? "Remove pairing" : "Pair",
                                 device.isTrusted ? "Untrust"
                                                  : "Trust (auto-connect)",
                                 "Add to favorites", "Back"};

  UI::printMenu(actions, 5);

  int choice = UI::promptChoice("Action:", 1, 5);

  switch (choice) {
  case 1: // Connect/Disconnect
    if (device.isConnected) {
      manager.disconnectDevice(device.macAddress);
      device.isConnected = false;
    } else {
      if (!device.isPaired) {
        manager.pairDevice(device.macAddress);
        device.isPaired = true;
      }
      if (manager.connectDevice(device.macAddress)) {
        device.isConnected = true;
      }
    }
    break;

  case 2: // Pair/Remove
    if (device.isPaired) {
      manager.removeDevice(device.macAddress);
      device.isPaired = false;
    } else {
      if (manager.pairDevice(device.macAddress)) {
        device.isPaired = true;
      }
    }
    break;

  case 3: // Trust/Untrust
    if (device.isTrusted) {
      // Untrust not directly supported, would need block/unblock
      UI::printWarning("Use 'block' to prevent auto-connect");
    } else {
      manager.trustDevice(device.macAddress);
      device.isTrusted = true;
      UI::printSuccess("Device trusted - will auto-connect");
    }
    break;

  case 4: // Favorites
    manager.getHistory().addFavorite(device.macAddress);
    UI::printSuccess("Added to favorites!");
    break;

  case 5: // Back
    break;
  }
}

/**
 * @brief Quick connect menu (favorites)
 */
void quickConnectMenu(BluetoothManager &manager) {
  auto &favorites = manager.getHistory().getFavorites();

  if (favorites.empty()) {
    UI::printWarning("No favorites yet. Add devices from the scan menu.");
    return;
  }

  UI::printInfo("Favorites:");
  UI::printDivider();

  for (size_t i = 0; i < favorites.size(); i++) {
    const auto &d = favorites[i];
    UI::printDeviceEntry(i + 1, d.getDisplayName(), d.macAddress, d.isConnected,
                         d.isPaired);
  }

  std::cout << "  " << UI::Color::DIM << "[0] Cancel" << UI::Color::RESET
            << std::endl;
  std::cout << std::endl;

  int choice = UI::promptChoice("Quick connect to:", 0, favorites.size());

  if (choice > 0 && choice <= static_cast<int>(favorites.size())) {
    manager.connectDevice(favorites[choice - 1].macAddress);
  }
}

/**
 * @brief Adapter settings menu
 */
void adapterSettingsMenu(BluetoothManager &manager) {
  UI::printInfo("Adapter Settings");
  UI::printDivider();

  std::cout << manager.getAdapterInfo() << std::endl;

  const std::string items[] = {
      manager.isBluetoothOn() ? "Power OFF" : "Power ON", "Back"};

  UI::printMenu(items, 2);

  int choice = UI::promptChoice("Action:", 1, 2);

  if (choice == 1) {
    if (manager.isBluetoothOn()) {
      manager.powerOff();
      UI::printSuccess("Bluetooth powered off");
    } else {
      manager.powerOn();
      UI::printSuccess("Bluetooth powered on");
    }
  }
}

/**
 * @brief Main application entry point
 */
int main() {
  // Setup signal handler
  signal(SIGINT, signalHandler);
  signal(SIGTERM, signalHandler);

  try {
    // Initialize Bluetooth manager
    g_manager = std::make_unique<BluetoothManager>();

    // Unblock and power on Bluetooth
    g_manager->unblockAdapter();
    g_manager->powerOn();

  } catch (const BluetoothException &e) {
    UI::printError("Bluetooth initialization failed: " + std::string(e.what()));
    UI::printInfo("Make sure bluez is installed: sudo apt install bluez");
    return 1;
  }

  // Display header
  UI::printHeader("ToothDroid v2.0");
  UI::printInfo("Modern Linux Bluetooth Manager");

  // Main loop
  bool running = true;
  while (running) {
    showMainMenu();

    int choice = UI::promptChoice("Choice:", 1, 7);
    std::cout << std::endl;

    switch (choice) {
    case 1: { // Scan
      auto devices = g_manager->scanDevices(8);

      if (!devices.empty()) {
        int selected = deviceSelectionMenu(*g_manager);
        if (selected > 0) {
          auto *device = g_manager->selectDevice(selected - 1);
          if (device) {
            deviceActionMenu(*g_manager, *device);
          }
        }
      }
      break;
    }

    case 2: { // Paired devices
      auto paired = g_manager->getPairedDevices();
      if (paired.empty()) {
        UI::printWarning("No paired devices found.");
      } else {
        UI::printInfo("Paired Devices:");
        UI::printDivider();
        for (size_t i = 0; i < paired.size(); i++) {
          UI::printDeviceEntry(i + 1, paired[i].getDisplayName(),
                               paired[i].macAddress, paired[i].isConnected,
                               true);
        }
      }
      break;
    }

    case 3: { // Connect
      auto &devices = g_manager->getDiscoveredDevices();
      if (devices.empty()) {
        UI::printWarning("No devices in cache. Scanning first...");
        g_manager->scanDevices(5);
      }

      int selected = deviceSelectionMenu(*g_manager);
      if (selected > 0) {
        auto *device = g_manager->selectDevice(selected - 1);
        if (device) {
          if (!device->isPaired) {
            g_manager->pairDevice(device->macAddress);
          }
          g_manager->connectDevice(device->macAddress);
        }
      }
      break;
    }

    case 4: // Disconnect
      g_manager->disconnectDevice();
      break;

    case 5: // Quick connect
      quickConnectMenu(*g_manager);
      break;

    case 6: // Settings
      adapterSettingsMenu(*g_manager);
      break;

    case 7: // Exit
      running = false;
      break;
    }

    std::cout << std::endl;
  }

  UI::printInfo("Goodbye!");
  return 0;
}
