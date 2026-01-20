#ifndef TOOTHDROID_BLUETOOTH_MANAGER_H
#define TOOTHDROID_BLUETOOTH_MANAGER_H

#include <algorithm>
#include <array>
#include <cstdio>
#include <functional>
#include <memory>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "BluetoothDevice.h"
#include "UI.h"

namespace ToothDroid {

/**
 * @brief Exception class for Bluetooth operations
 */
class BluetoothException : public std::runtime_error {
public:
  explicit BluetoothException(const std::string &msg)
      : std::runtime_error(msg) {}
};

/**
 * @brief Manages Bluetooth operations using bluetoothctl
 *
 * Uses subprocess communication with bluetoothctl for D-Bus operations.
 * This is safer than raw system() calls as we capture and parse output.
 */
class BluetoothManager {
private:
  DeviceHistory history;
  std::string currentAdapter;
  std::vector<BluetoothDevice> discoveredDevices;
  BluetoothDevice *selectedDevice = nullptr;
  bool isScanning = false;

  /**
   * @brief Execute a command and capture its output
   */
  std::string executeCommand(const std::string &cmd) const {
    std::array<char, 128> buffer;
    std::string result;

    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
      throw BluetoothException("Failed to execute command: " + cmd);
    }

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
      result += buffer.data();
    }

    int exitCode = pclose(pipe);
    if (exitCode != 0) {
      // Not always an error - some commands return non-zero on normal
      // conditions
    }

    return result;
  }

  /**
   * @brief Execute bluetoothctl command
   */
  std::string bluetoothctl(const std::string &args) const {
    return executeCommand("bluetoothctl " + args + " 2>&1");
  }

  /**
   * @brief Parse device info from bluetoothctl output
   */
  BluetoothDevice parseDeviceInfo(const std::string &mac) const {
    BluetoothDevice device;
    device.macAddress = mac;

    std::string info = bluetoothctl("info " + mac);
    std::istringstream stream(info);
    std::string line;

    while (std::getline(stream, line)) {
      // Trim leading whitespace
      size_t start = line.find_first_not_of(" \t");
      if (start == std::string::npos)
        continue;
      line = line.substr(start);

      if (line.find("Name:") == 0) {
        device.name = line.substr(6);
      } else if (line.find("Alias:") == 0) {
        device.alias = line.substr(7);
      } else if (line.find("Paired:") == 0) {
        device.isPaired = (line.find("yes") != std::string::npos);
      } else if (line.find("Connected:") == 0) {
        device.isConnected = (line.find("yes") != std::string::npos);
      } else if (line.find("Trusted:") == 0) {
        device.isTrusted = (line.find("yes") != std::string::npos);
      } else if (line.find("Blocked:") == 0) {
        device.isBlocked = (line.find("yes") != std::string::npos);
      } else if (line.find("Icon:") == 0) {
        device.icon = line.substr(6);
      } else if (line.find("UUID: Audio Sink") != std::string::npos) {
        device.supportsA2DP = true;
      } else if (line.find("UUID: Headset") != std::string::npos) {
        device.supportsHSP = true;
      } else if (line.find("UUID: Handsfree") != std::string::npos) {
        device.supportsHFP = true;
      }
    }

    device.lastSeen = std::time(nullptr);
    return device;
  }

public:
  BluetoothManager() {
    // Check if bluetoothctl is available
    std::string version = executeCommand("bluetoothctl --version 2>&1");
    if (version.find("bluetoothctl") == std::string::npos) {
      throw BluetoothException("bluetoothctl not found. Please install bluez.");
    }
  }

  /**
   * @brief Unblock Bluetooth adapter
   */
  bool unblockAdapter() {
    std::string result = executeCommand("rfkill unblock bluetooth 2>&1");
    return true;
  }

  /**
   * @brief Power on the Bluetooth adapter
   */
  bool powerOn() {
    std::string result = bluetoothctl("power on");
    return result.find("succeeded") != std::string::npos;
  }

  /**
   * @brief Power off the Bluetooth adapter
   */
  bool powerOff() {
    std::string result = bluetoothctl("power off");
    return result.find("succeeded") != std::string::npos;
  }

  /**
   * @brief Start scanning for devices
   * @param duration Scan duration in seconds
   * @return Vector of discovered devices
   */
  std::vector<BluetoothDevice> scanDevices(int duration = 10) {
    discoveredDevices.clear();

    UI::printStep("Starting Bluetooth scan...");

    // Power on adapter
    powerOn();

    // Start scan
    bluetoothctl("scan on &");

    // Wait for scan duration with progress
    for (int i = 0; i < duration; i++) {
      UI::printProgress(i + 1, duration, "Scanning");
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Stop scan
    bluetoothctl("scan off");

    // Get list of devices
    std::string output = bluetoothctl("devices");
    std::istringstream stream(output);
    std::string line;

    // Parse "Device XX:XX:XX:XX:XX:XX Name" format
    std::regex deviceRegex(R"(Device\s+([0-9A-Fa-f:]{17})\s+(.+))");
    std::smatch match;

    while (std::getline(stream, line)) {
      if (std::regex_search(line, match, deviceRegex)) {
        std::string mac = match[1];
        std::string name = match[2];

        BluetoothDevice device = parseDeviceInfo(mac);
        if (device.name.empty()) {
          device.name = name;
        }

        discoveredDevices.push_back(device);
        history.addDevice(device);
      }
    }

    // Sort by signal strength / paired status
    std::sort(discoveredDevices.begin(), discoveredDevices.end(),
              [](const BluetoothDevice &a, const BluetoothDevice &b) {
                if (a.isConnected != b.isConnected)
                  return a.isConnected;
                if (a.isPaired != b.isPaired)
                  return a.isPaired;
                return a.name < b.name;
              });

    UI::printSuccess("Found " + std::to_string(discoveredDevices.size()) +
                     " device(s)");

    return discoveredDevices;
  }

  /**
   * @brief Get list of paired devices
   */
  std::vector<BluetoothDevice> getPairedDevices() {
    std::vector<BluetoothDevice> devices;
    std::string output = bluetoothctl("paired-devices");
    std::istringstream stream(output);
    std::string line;

    std::regex deviceRegex(R"(Device\s+([0-9A-Fa-f:]{17})\s+(.+))");
    std::smatch match;

    while (std::getline(stream, line)) {
      if (std::regex_search(line, match, deviceRegex)) {
        BluetoothDevice device = parseDeviceInfo(match[1]);
        if (device.name.empty()) {
          device.name = match[2];
        }
        devices.push_back(device);
      }
    }

    return devices;
  }

  /**
   * @brief Pair with a device
   */
  bool pairDevice(const std::string &mac) {
    UI::printStep("Pairing with " + mac + "...");

    std::string result = bluetoothctl("pair " + mac);

    if (result.find("Pairing successful") != std::string::npos ||
        result.find("already paired") != std::string::npos) {
      UI::printSuccess("Paired successfully!");

      // Auto-trust for convenience
      bluetoothctl("trust " + mac);

      return true;
    }

    UI::printError("Pairing failed: " + result);
    return false;
  }

  /**
   * @brief Connect to a device
   */
  bool connectDevice(const std::string &mac) {
    UI::printStep("Connecting to " + mac + "...");

    std::string result = bluetoothctl("connect " + mac);

    if (result.find("Connection successful") != std::string::npos ||
        result.find("already connected") != std::string::npos) {
      UI::printSuccess("Connected successfully!");

      // Update device in history
      BluetoothDevice *dev = history.findDevice(mac);
      if (dev) {
        dev->isConnected = true;
        dev->lastConnected = std::time(nullptr);
      }

      return true;
    }

    UI::printError("Connection failed: " + result);
    return false;
  }

  /**
   * @brief Disconnect from a device
   */
  bool disconnectDevice(const std::string &mac = "") {
    std::string target = mac;

    if (target.empty()) {
      // Disconnect all
      UI::printStep("Disconnecting all devices...");
      bluetoothctl("disconnect");
    } else {
      UI::printStep("Disconnecting " + target + "...");
      bluetoothctl("disconnect " + target);
    }

    UI::printSuccess("Disconnected");
    return true;
  }

  /**
   * @brief Remove (unpair) a device
   */
  bool removeDevice(const std::string &mac) {
    UI::printStep("Removing " + mac + "...");

    std::string result = bluetoothctl("remove " + mac);

    if (result.find("Device has been removed") != std::string::npos) {
      UI::printSuccess("Device removed");
      return true;
    }

    UI::printWarning("Could not remove device");
    return false;
  }

  /**
   * @brief Trust a device (allows auto-connect)
   */
  bool trustDevice(const std::string &mac) {
    std::string result = bluetoothctl("trust " + mac);
    return result.find("succeeded") != std::string::npos ||
           result.find("already trusted") != std::string::npos;
  }

  /**
   * @brief Block a device
   */
  bool blockDevice(const std::string &mac) {
    std::string result = bluetoothctl("block " + mac);
    return result.find("succeeded") != std::string::npos;
  }

  /**
   * @brief Unblock a device
   */
  bool unblockDevice(const std::string &mac) {
    std::string result = bluetoothctl("unblock " + mac);
    return result.find("succeeded") != std::string::npos;
  }

  /**
   * @brief Get adapter info
   */
  std::string getAdapterInfo() { return bluetoothctl("show"); }

  /**
   * @brief Check if Bluetooth is powered on
   */
  bool isBluetoothOn() {
    std::string info = getAdapterInfo();
    return info.find("Powered: yes") != std::string::npos;
  }

  /**
   * @brief Get discovered devices (from last scan)
   */
  const std::vector<BluetoothDevice> &getDiscoveredDevices() const {
    return discoveredDevices;
  }

  /**
   * @brief Get device history
   */
  DeviceHistory &getHistory() { return history; }

  /**
   * @brief Select a device by index (from discovered list)
   */
  BluetoothDevice *selectDevice(int index) {
    if (index >= 0 && index < static_cast<int>(discoveredDevices.size())) {
      selectedDevice = &discoveredDevices[index];
      return selectedDevice;
    }
    return nullptr;
  }

  /**
   * @brief Get currently selected device
   */
  BluetoothDevice *getSelectedDevice() { return selectedDevice; }

  /**
   * @brief Display discovered devices in a formatted list
   */
  void displayDevices() {
    if (discoveredDevices.empty()) {
      UI::printWarning("No devices found. Try scanning first.");
      return;
    }

    std::cout << std::endl;
    UI::printInfo("Available Devices:");
    UI::printDivider();

    for (size_t i = 0; i < discoveredDevices.size(); i++) {
      const auto &d = discoveredDevices[i];
      UI::printDeviceEntry(i + 1, d.getDisplayName(), d.macAddress,
                           d.isConnected, d.isPaired);
    }

    UI::printDivider();
    std::cout << "  " << UI::Color::DIM << "● Connected  ○ Paired  ◌ Available"
              << UI::Color::RESET << std::endl;
  }
};

} // namespace ToothDroid

#endif // TOOTHDROID_BLUETOOTH_MANAGER_H
