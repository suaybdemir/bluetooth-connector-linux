#ifndef TOOTHDROID_BLUETOOTH_DEVICE_H
#define TOOTHDROID_BLUETOOTH_DEVICE_H

#include <ctime>
#include <string>
#include <vector>

namespace ToothDroid {

/**
 * @brief Represents a Bluetooth device discovered or known to the system
 */
struct BluetoothDevice {
  std::string name;              // Device friendly name
  std::string macAddress;        // MAC address (XX:XX:XX:XX:XX:XX format)
  std::string alias;             // Custom alias set by user
  bool isPaired = false;         // Whether device is paired
  bool isConnected = false;      // Current connection status
  bool isTrusted = false;        // Whether device is trusted (auto-connect)
  bool isBlocked = false;        // Whether device is blocked
  int16_t rssi = 0;              // Signal strength (dBm)
  std::string deviceClass;       // Device class (phone, headset, etc.)
  std::string icon;              // Icon type for display
  std::time_t lastSeen = 0;      // Last time device was seen
  std::time_t lastConnected = 0; // Last successful connection

  // Audio-specific
  bool supportsA2DP = false; // Advanced Audio Distribution Profile
  bool supportsHSP = false;  // Headset Profile
  bool supportsHFP = false;  // Hands-Free Profile

  // Equality based on MAC address
  bool operator==(const BluetoothDevice &other) const {
    return macAddress == other.macAddress;
  }

  // For sorting (by signal strength, then name)
  bool operator<(const BluetoothDevice &other) const {
    if (rssi != other.rssi)
      return rssi > other.rssi; // Higher RSSI first
    return name < other.name;
  }

  // Check if device supports audio
  bool hasAudioSupport() const {
    return supportsA2DP || supportsHSP || supportsHFP;
  }

  // Get display name (alias if set, otherwise name, otherwise MAC)
  std::string getDisplayName() const {
    if (!alias.empty())
      return alias;
    if (!name.empty())
      return name;
    return macAddress;
  }

  // Get status icon
  std::string getStatusIcon() const {
    if (isConnected)
      return "●"; // Connected
    if (isPaired)
      return "○"; // Paired but not connected
    return "◌";   // Discovered only
  }
};

/**
 * @brief In-memory device history storage
 */
class DeviceHistory {
private:
  std::vector<BluetoothDevice> knownDevices;
  std::vector<BluetoothDevice> favoriteDevices;
  size_t maxHistorySize = 50;

public:
  // Add or update a device in history
  void addDevice(const BluetoothDevice &device) {
    for (auto &d : knownDevices) {
      if (d.macAddress == device.macAddress) {
        // Update existing
        d = device;
        return;
      }
    }
    // Add new
    if (knownDevices.size() >= maxHistorySize) {
      knownDevices.erase(knownDevices.begin());
    }
    knownDevices.push_back(device);
  }

  // Get device by MAC
  BluetoothDevice *findDevice(const std::string &mac) {
    for (auto &d : knownDevices) {
      if (d.macAddress == mac)
        return &d;
    }
    return nullptr;
  }

  // Get all known devices
  const std::vector<BluetoothDevice> &getKnownDevices() const {
    return knownDevices;
  }

  // Get paired devices only
  std::vector<BluetoothDevice> getPairedDevices() const {
    std::vector<BluetoothDevice> result;
    for (const auto &d : knownDevices) {
      if (d.isPaired)
        result.push_back(d);
    }
    return result;
  }

  // Add to favorites
  void addFavorite(const std::string &mac) {
    auto *device = findDevice(mac);
    if (device) {
      for (const auto &f : favoriteDevices) {
        if (f.macAddress == mac)
          return; // Already favorite
      }
      favoriteDevices.push_back(*device);
    }
  }

  // Get favorites
  const std::vector<BluetoothDevice> &getFavorites() const {
    return favoriteDevices;
  }

  // Clear all history
  void clear() {
    knownDevices.clear();
    favoriteDevices.clear();
  }

  // Get history size
  size_t size() const { return knownDevices.size(); }
};

} // namespace ToothDroid

#endif // TOOTHDROID_BLUETOOTH_DEVICE_H
