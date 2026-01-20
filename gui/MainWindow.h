/**
 * @file gui/MainWindow.h
 * @brief Main application window for ToothDroid GTK4 GUI
 */

#ifndef TOOTHDROID_GUI_MAINWINDOW_H
#define TOOTHDROID_GUI_MAINWINDOW_H

#include <atomic>
#include <gtkmm.h>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "../include/BluetoothDevice.h"
#include "../include/BluetoothManager.h"

namespace ToothDroid {
namespace GUI {

/**
 * @brief Custom widget for displaying a single Bluetooth device
 */
class DeviceRow : public Gtk::Box {
public:
  DeviceRow(const BluetoothDevice &device);

  sigc::signal<void(const std::string &)> signal_connect_clicked();
  sigc::signal<void(const std::string &)> signal_disconnect_clicked();

  void updateStatus(bool connected, bool paired);
  const std::string &getMacAddress() const { return macAddress; }

private:
  std::string macAddress;
  Gtk::Image *statusIcon;
  Gtk::Label *nameLabel;
  Gtk::Label *macLabel;
  Gtk::Label *profileLabel;
  Gtk::Button *actionButton;
  bool isConnected = false;

  sigc::signal<void(const std::string &)> m_signal_connect;
  sigc::signal<void(const std::string &)> m_signal_disconnect;

  void onActionClicked();
};

/**
 * @brief Main application window
 */
class MainWindow : public Gtk::ApplicationWindow {
public:
  MainWindow(); // Default constructor
  virtual ~MainWindow();

  static MainWindow *create();

protected:
  // UI Components
  Gtk::HeaderBar *headerBar;
  Gtk::Button *scanButton;
  Gtk::Button *settingsButton;
  Gtk::ProgressBar *scanProgress;
  Gtk::Stack *mainStack;
  Gtk::ListBox *deviceList;
  Gtk::Label *statusLabel;
  Gtk::Spinner *loadingSpinner;
  Gtk::Box *emptyState;
  Gtk::ScrolledWindow *scrolledDevices;

  // Bluetooth manager
  std::unique_ptr<BluetoothManager> btManager;

  // Scan thread control
  std::thread scanThread;
  std::atomic<bool> scanning{false};

  // Signal handlers
  void onScanClicked();
  void onSettingsClicked();
  void onDeviceConnect(const std::string &mac);
  void onDeviceDisconnect(const std::string &mac);

  // Helper methods
  void startScan();
  void stopScan();
  void updateDeviceList(const std::vector<BluetoothDevice> &devices);
  void showNotification(const std::string &message, bool isError = false);
  void setScanning(bool state);

  // Dispatcher for thread-safe UI updates
  Glib::Dispatcher dispatcher;
  std::vector<BluetoothDevice> pendingDevices;
  std::mutex devicesMutex;

  void onDispatcherSignal();
};

/**
 * @brief Application class
 */
class ToothDroidApp : public Gtk::Application {
public:
  static Glib::RefPtr<ToothDroidApp> create();

protected:
  ToothDroidApp();

  void on_activate() override;
  void on_startup() override;

private:
  MainWindow *mainWindow = nullptr;

  void createWindow();
  void applyCSS();
};

} // namespace GUI
} // namespace ToothDroid

#endif // TOOTHDROID_GUI_MAINWINDOW_H
