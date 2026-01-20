# Changelog

All notable changes to this project will be documented in this file.

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
