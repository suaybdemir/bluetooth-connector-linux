#ifndef TOOTHDROID_UI_H
#define TOOTHDROID_UI_H

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

namespace UI {

// ANSI Color Codes
namespace Color {
constexpr const char *RESET = "\033[0m";
constexpr const char *BOLD = "\033[1m";
constexpr const char *DIM = "\033[2m";

// Foreground
constexpr const char *RED = "\033[31m";
constexpr const char *GREEN = "\033[32m";
constexpr const char *YELLOW = "\033[33m";
constexpr const char *BLUE = "\033[34m";
constexpr const char *MAGENTA = "\033[35m";
constexpr const char *CYAN = "\033[36m";
constexpr const char *WHITE = "\033[37m";

// Bright
constexpr const char *BRIGHT_RED = "\033[91m";
constexpr const char *BRIGHT_GREEN = "\033[92m";
constexpr const char *BRIGHT_YELLOW = "\033[93m";
constexpr const char *BRIGHT_BLUE = "\033[94m";
constexpr const char *BRIGHT_CYAN = "\033[96m";

// Background
constexpr const char *BG_BLUE = "\033[44m";
constexpr const char *BG_GREEN = "\033[42m";
} // namespace Color

// Output helpers
inline void printSuccess(const std::string &msg) {
  std::cout << Color::GREEN << "✓ " << msg << Color::RESET << std::endl;
}

inline void printError(const std::string &msg) {
  std::cout << Color::RED << "✗ " << msg << Color::RESET << std::endl;
}

inline void printWarning(const std::string &msg) {
  std::cout << Color::YELLOW << "⚠ " << msg << Color::RESET << std::endl;
}

inline void printInfo(const std::string &msg) {
  std::cout << Color::CYAN << "ℹ " << msg << Color::RESET << std::endl;
}

inline void printStep(const std::string &msg) {
  std::cout << Color::BLUE << "➤ " << msg << Color::RESET << std::endl;
}

inline void printHeader(const std::string &title) {
  std::cout << std::endl;
  std::cout << Color::BOLD << Color::BRIGHT_CYAN;
  std::cout << "╔════════════════════════════════════════════════╗"
            << std::endl;
  std::cout << "║  " << Color::BRIGHT_BLUE << "🔵 " << title;
  // Pad to fit the box
  size_t padding = 44 - title.length();
  for (size_t i = 0; i < padding; i++)
    std::cout << " ";
  std::cout << Color::BRIGHT_CYAN << "║" << std::endl;
  std::cout << "╚════════════════════════════════════════════════╝";
  std::cout << Color::RESET << std::endl;
  std::cout << std::endl;
}

inline void printDivider() {
  std::cout << Color::DIM << "────────────────────────────────────────────────"
            << Color::RESET << std::endl;
}

inline void printMenu(const std::string items[], int count) {
  std::cout << std::endl;
  std::cout << Color::BOLD << Color::WHITE
            << " Select an option:" << Color::RESET << std::endl;
  std::cout << std::endl;

  for (int i = 0; i < count; i++) {
    std::cout << "  " << Color::BRIGHT_CYAN << "[" << (i + 1) << "]"
              << Color::RESET << " " << items[i] << std::endl;
  }
  std::cout << std::endl;
}

// Spinner for loading operations
class Spinner {
private:
  const char *frames[4] = {"⠋", "⠙", "⠹", "⠸"};
  int current = 0;
  std::string message;
  bool running = false;

public:
  void start(const std::string &msg) {
    message = msg;
    running = true;
  }

  void update() {
    if (!running)
      return;
    std::cout << "\r" << Color::CYAN << frames[current] << " " << message
              << Color::RESET << std::flush;
    current = (current + 1) % 4;
  }

  void stop(bool success = true) {
    running = false;
    std::cout << "\r";
    if (success) {
      printSuccess(message);
    } else {
      printError(message);
    }
  }
};

// Progress bar
inline void printProgress(int current, int total,
                          const std::string &label = "") {
  const int barWidth = 30;
  float progress = static_cast<float>(current) / total;
  int filled = static_cast<int>(progress * barWidth);

  std::cout << "\r" << Color::CYAN << label << " [";
  for (int i = 0; i < barWidth; i++) {
    if (i < filled)
      std::cout << "█";
    else
      std::cout << "░";
  }
  std::cout << "] " << static_cast<int>(progress * 100) << "%" << Color::RESET
            << std::flush;

  if (current == total)
    std::cout << std::endl;
}

// Device display formatting
inline void printDeviceEntry(int index, const std::string &name,
                             const std::string &mac, bool connected = false,
                             bool paired = false) {
  std::cout << "  " << Color::BRIGHT_CYAN << "[" << index << "]"
            << Color::RESET;
  std::cout << " " << Color::BOLD << name << Color::RESET;
  std::cout << " " << Color::DIM << "(" << mac << ")" << Color::RESET;

  if (connected) {
    std::cout << " " << Color::GREEN << "●" << Color::RESET;
  } else if (paired) {
    std::cout << " " << Color::YELLOW << "○" << Color::RESET;
  }
  std::cout << std::endl;
}

// Prompt for input
inline int promptChoice(const std::string &prompt, int min, int max) {
  int choice;
  while (true) {
    std::cout << Color::BRIGHT_YELLOW << prompt << Color::RESET << " ";
    if (std::cin >> choice) {
      if (choice >= min && choice <= max) {
        return choice;
      }
    }
    std::cin.clear();
    std::cin.ignore(10000, '\n');
    printError("Invalid choice. Please try again.");
  }
}

inline std::string promptString(const std::string &prompt) {
  std::string input;
  std::cout << Color::BRIGHT_YELLOW << prompt << Color::RESET << " ";
  std::cin >> input;
  return input;
}

inline bool promptConfirm(const std::string &prompt) {
  std::cout << Color::BRIGHT_YELLOW << prompt << " [y/N]" << Color::RESET
            << " ";
  char c;
  std::cin >> c;
  return (c == 'y' || c == 'Y');
}

} // namespace UI

#endif // TOOTHDROID_UI_H
