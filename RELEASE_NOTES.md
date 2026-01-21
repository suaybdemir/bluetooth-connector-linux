# 🦷 ToothDroid v2.2 Release Notes

**ToothDroid v2.2** marks a major milestone in the project's evolution, featuring a complete rewrite of the GUI using the **Qt6 framework** and significant user experience improvements.

## 🚀 Major Changes

### 🎨 Qt6 GUI Migration
- **Complete Rewrite:** The application interface has been entirely migrated from GTK4 to **Qt6**, offering better stability and a modern look.
- **Mac-Style UI:** Introduced a custom, frameless window design with macOS-inspired "traffic light" window controls (Close, Minimize, Maximize).
- **Responsive Layout:** Improved layout management for better resizing behavior.

### 🖱️ Enhanced Interaction
- **Expanded Context Menu:** Right-click on any device to access a full suite of management options:
  - **Connect / Disconnect**
  - **Pair / Unpair (Forget)**
  - **Trust / Block**
  - **Device Info**
- **Auto-Scan:** Bluetooth scanning now activates automatically upon launch (with a 200ms stabilization delay).

### 🛠️ Stability & Performance
- **Thread Safety:** Replaced `QtConcurrent` with standard C++ `std::thread` for reliable asynchronous Bluetooth operations.
- **Crash Prevention:** Implemented `QPointer` smart pointers to guard against SegFaults during device updates.
- **Signal Cleanup:** Removed legacy signal handlers to improve application shutdown reliability.

## ℹ️ Other Improvements
- **About Dialog:** Added a new "About" dialog accessible via the settings button for version and license info.
- **Version Awareness:** Window title now correctly identifies as `ToothDroid v2.2`.

## 📦 Upgrading
Rebuild the project to apply the new Qt6 changes:
```bash
git pull
make clean
make
```

---
*Happy Connecting!* 🦷🤖
