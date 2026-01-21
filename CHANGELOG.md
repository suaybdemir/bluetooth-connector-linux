# Changelog

All notable changes to this project will be documented in this file.

## [v2.2.0] - 2026-01-21

### 💥 Breaking Changes

- **Qt6 Migration:** Replaced the entire GTK4 GUI stack with **Qt6** (QtWidgets) for improved cross-platform support and stability.

### 🚀 Features

- **Mac-Style UI:** Implemented custom traffic-light window controls (Close/Min/Max) and a frameless window design.
- **Context Menu:** Added a comprehensive right-click menu for devices (Trust, Block, Pair, Unpair, Connect, Disconnect).
- **Auto-Scan:** Enabled automatic background scanning on startup.
- **About Dialog:** Added a Qt-based About dialog.

### 🐛 Fixes

- **Stability:** Replaced `QtConcurrent` with `std::thread` and added `QPointer` guards to fix Segmentation Faults.
- **Race Components:** Resolved data races in device discovery and removal.

## [v2.1.0] - 2026-01-20

### 🚀 Features

- **About Dialog:** Added a native `Gtk::AboutDialog` triggered by the settings button.
- **Identification:** Updated window title and version strings to v2.1.

### 🧹 Maintenance

- **Code Quality:** Refactored dialog signal handling for GTK4 compliance.

## [v2.0.0] - 2026-01-20

### 🚀 Features

- **New UI:** Complete redesign with a modern, macOS-inspired Dark Mode aesthetic.
- **Unified Header:** Clean, integrated header bar with system controls.
- **Visual Feedback:** 
  - Real-time connection status indicators (Green/Blue/Gray).
  - Animated scanning progress.
- **Auto-Scan:** Application now automatically scans for devices on startup (500ms delay).
- **Settings Access:** Added a dedicated settings button for quick access to configuration.

### 🐛 Fixes

- **Layout Issues:** Resolved white box artifacts on window controls caused by generic CSS selectors.
- **Theme Consistency:** Fixed status bar background color mismatch.
- **Visibility:** Ensured "Scan" and "Settings" buttons are always visible and accessible.
- **Thread Safety:** Implemented `Glib::Dispatcher` for thread-safe UI updates from background threads.

### 🛠️ Tech Stack Changes

- Migrated to **GTK4** and **gtkmm-4.0** for better performance and modern widget set.
- styling moved to pure **CSS** with a high-priority provider to override system themes.
