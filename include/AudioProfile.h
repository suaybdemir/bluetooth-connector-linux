#ifndef TOOTHDROID_AUDIO_PROFILE_H
#define TOOTHDROID_AUDIO_PROFILE_H

#include <array>
#include <cstdio>
#include <sstream>
#include <string>
#include <vector>

#include "UI.h"

namespace ToothDroid {

/**
 * @brief Audio profile types supported by Bluetooth
 */
enum class AudioProfileType {
  A2DP, // Advanced Audio Distribution Profile (high quality audio)
  HSP,  // Headset Profile (voice calls)
  HFP,  // Hands-Free Profile (car kits)
  AVRCP // Audio/Video Remote Control Profile
};

/**
 * @brief Manages audio profiles and PulseAudio/PipeWire integration
 */
class AudioManager {
private:
  bool usePipeWire = false;

  std::string executeCommand(const std::string &cmd) const {
    std::array<char, 128> buffer;
    std::string result;
    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe)
      return "";
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
      result += buffer.data();
    }
    pclose(pipe);
    return result;
  }

public:
  AudioManager() {
    // Detect if we're using PipeWire or PulseAudio
    std::string result = executeCommand("pactl info 2>&1");
    usePipeWire = (result.find("PipeWire") != std::string::npos);
  }

  /**
   * @brief Check if using PipeWire
   */
  bool isPipeWire() const { return usePipeWire; }

  /**
   * @brief Get audio server name
   */
  std::string getAudioServer() const {
    return usePipeWire ? "PipeWire" : "PulseAudio";
  }

  /**
   * @brief List available audio sinks (outputs)
   */
  std::vector<std::string> getAudioSinks() {
    std::vector<std::string> sinks;
    std::string output = executeCommand("pactl list sinks short");
    std::istringstream stream(output);
    std::string line;

    while (std::getline(stream, line)) {
      if (!line.empty()) {
        sinks.push_back(line);
      }
    }
    return sinks;
  }

  /**
   * @brief List available audio sources (inputs)
   */
  std::vector<std::string> getAudioSources() {
    std::vector<std::string> sources;
    std::string output = executeCommand("pactl list sources short");
    std::istringstream stream(output);
    std::string line;

    while (std::getline(stream, line)) {
      if (!line.empty()) {
        sources.push_back(line);
      }
    }
    return sources;
  }

  /**
   * @brief Set default audio sink to Bluetooth device
   */
  bool setBluetoothAsSink(const std::string &sinkName) {
    std::string cmd = "pactl set-default-sink \"" + sinkName + "\"";
    std::string result = executeCommand(cmd + " 2>&1");
    return result.empty(); // Empty output means success
  }

  /**
   * @brief Set default audio source to Bluetooth device
   */
  bool setBluetoothAsSource(const std::string &sourceName) {
    std::string cmd = "pactl set-default-source \"" + sourceName + "\"";
    std::string result = executeCommand(cmd + " 2>&1");
    return result.empty();
  }

  /**
   * @brief Get Bluetooth audio sink by MAC address
   */
  std::string findBluetoothSink(const std::string &mac) {
    // Convert MAC to format used in sink names
    std::string macFormatted = mac;
    for (char &c : macFormatted) {
      if (c == ':')
        c = '_';
    }

    auto sinks = getAudioSinks();
    for (const auto &sink : sinks) {
      if (sink.find(macFormatted) != std::string::npos ||
          sink.find("bluez") != std::string::npos) {
        // Extract sink name (second column in pactl output)
        std::istringstream iss(sink);
        std::string id, name;
        iss >> id >> name;
        return name;
      }
    }
    return "";
  }

  /**
   * @brief Set volume for a sink
   */
  bool setVolume(const std::string &sinkName, int percent) {
    if (percent < 0)
      percent = 0;
    if (percent > 150)
      percent = 150;

    std::string cmd = "pactl set-sink-volume \"" + sinkName + "\" " +
                      std::to_string(percent) + "%";
    executeCommand(cmd);
    return true;
  }

  /**
   * @brief Mute/unmute a sink
   */
  bool setMute(const std::string &sinkName, bool mute) {
    std::string cmd =
        "pactl set-sink-mute \"" + sinkName + "\" " + (mute ? "1" : "0");
    executeCommand(cmd);
    return true;
  }

  /**
   * @brief Switch audio profile for a Bluetooth device
   * Profile can be: a2dp_sink, headset_head_unit, off
   */
  bool setCardProfile(const std::string &mac, const std::string &profile) {
    std::string macFormatted = mac;
    for (char &c : macFormatted) {
      if (c == ':')
        c = '_';
    }

    // Find the card
    std::string cards = executeCommand("pactl list cards short");
    std::istringstream stream(cards);
    std::string line;
    std::string cardName;

    while (std::getline(stream, line)) {
      if (line.find(macFormatted) != std::string::npos ||
          line.find("bluez") != std::string::npos) {
        std::istringstream iss(line);
        std::string id;
        iss >> id >> cardName;
        break;
      }
    }

    if (cardName.empty()) {
      return false;
    }

    std::string cmd = "pactl set-card-profile \"" + cardName + "\" " + profile;
    std::string result = executeCommand(cmd + " 2>&1");
    return result.empty();
  }

  /**
   * @brief Switch to A2DP high quality audio
   */
  bool switchToA2DP(const std::string &mac) {
    UI::printStep("Switching to A2DP (high quality audio)...");
    if (setCardProfile(mac, "a2dp_sink")) {
      UI::printSuccess("Switched to A2DP");
      return true;
    }
    UI::printError("Failed to switch profile");
    return false;
  }

  /**
   * @brief Switch to HSP/HFP for voice calls
   */
  bool switchToHeadset(const std::string &mac) {
    UI::printStep("Switching to Headset profile (voice)...");
    if (setCardProfile(mac, "headset_head_unit")) {
      UI::printSuccess("Switched to Headset profile");
      return true;
    }
    UI::printError("Failed to switch profile");
    return false;
  }

  /**
   * @brief Display audio status
   */
  void displayStatus() {
    UI::printInfo("Audio Server: " + getAudioServer());
    UI::printDivider();

    std::cout << UI::Color::CYAN << "Audio Outputs:" << UI::Color::RESET
              << std::endl;
    for (const auto &sink : getAudioSinks()) {
      bool isBluetooth = (sink.find("bluez") != std::string::npos);
      if (isBluetooth) {
        std::cout << "  " << UI::Color::BLUE << "🔵 " << UI::Color::RESET;
      } else {
        std::cout << "  " << UI::Color::DIM << "  " << UI::Color::RESET;
      }
      std::cout << sink << std::endl;
    }
  }
};

} // namespace ToothDroid

#endif // TOOTHDROID_AUDIO_PROFILE_H
