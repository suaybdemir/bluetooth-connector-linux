# ToothDroid Project Summary

## 🚀 The Value Proposition

**Problem:** Bluetooth management on Linux is fragmented. Users often have to choose between cryptic command-line tools (`bluetoothctl`) that are hard to memorize, or heavy desktop environment-specific tools (GNOME Bluetooth, Bluedevil) that bring in hundreds of megabytes of dependencies. Connecting a simple pair of headphones shouldn't require a web search for commands or a full DE installation.

**Solution:** ToothDroid bridges this gap. It's a lightweight, native C++ application that provides:
1.  **A clean GUI** for users who just want things to work (click-to-connect).
2.  **A powerful CLI** for minimalists or server environments, with an interactive menu (no command memorization needed).
3.  **Zero heavy dependencies**: It runs on any distro (Arch, Fedora, Ubuntu) without pulling in GNOME or KDE libraries.

## 🎯 Target Audience
- **Arch/Gentoo/Void users** who build their own minimal desktop environments (i3, Sway, Hyprland) and need a standalone bluetooth manager.
- **Server administrators** who need to quickly pair devices on headless machines using the easy CLI menu.
- **Developers** looking for a reference implementation of BlueZ D-Bus communication in Modern C++.

## 🔑 Key Features
- **Visual Feedback**: Instantly see if a device is connected (Green) or just paired (Blue).
- **Audio Intelligence**: Automatically detects and displays audio capabilities (A2DP, HSP, HFP).
- **Hybrid Workflow**: Use the CLI for quick tasks, use the GUI for visual monitoring.
- **Speed**: Written in C++17, it starts instantly and uses negligible RAM.

## 🛠️ Technical Stack
- **Languages**: C++17
- **GUI Framework**: Qt6 (LGPL) - Chosen for cross-platform robustness and modern styling.
- **Backend**: BlueZ D-Bus API (via `bluetoothctl` wrapper for stability).
- **Build System**: GNU Make - Simple, standard, and universally available.

## 🤝 Call for Contributions
We are actively looking for help with:
1.  **Distro Packaging**: creating AUR packages, .deb/.rpm files.
2.  **Localization**: Translating the UI into more languages.
3.  **Testing**: Verifying functionality on different Bluetooth hardware.

Join us in making Bluetooth on Linux simple again!
