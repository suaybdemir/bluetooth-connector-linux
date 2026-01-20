/**
 * @file gui/MainWindow.cpp
 * @brief Implementation of ToothDroid GTK4 GUI
 */

#include "MainWindow.h"
#include <iostream>

namespace ToothDroid {
namespace GUI {

// ============================================================================
// DeviceRow Implementation
// ============================================================================

DeviceRow::DeviceRow(const BluetoothDevice &device)
    : Gtk::Box(Gtk::Orientation::HORIZONTAL, 12), macAddress(device.macAddress),
      isConnected(device.isConnected) {
  set_margin(12);
  add_css_class("device-row");

  // Status icon
  statusIcon = Gtk::make_managed<Gtk::Image>();
  if (device.isConnected) {
    statusIcon->set_from_icon_name("bluetooth-active-symbolic");
    add_css_class("connected");
  } else if (device.isPaired) {
    statusIcon->set_from_icon_name("bluetooth-symbolic");
    add_css_class("paired");
  } else {
    statusIcon->set_from_icon_name("bluetooth-disabled-symbolic");
  }
  append(*statusIcon);

  // Info box
  auto *infoBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 4);
  infoBox->set_hexpand(true);

  // Device name
  nameLabel = Gtk::make_managed<Gtk::Label>(device.getDisplayName());
  nameLabel->set_halign(Gtk::Align::START);
  nameLabel->add_css_class("device-name");
  infoBox->append(*nameLabel);

  // MAC address
  macLabel = Gtk::make_managed<Gtk::Label>(device.macAddress);
  macLabel->set_halign(Gtk::Align::START);
  macLabel->add_css_class("device-mac");
  macLabel->add_css_class("dim-label");
  infoBox->append(*macLabel);

  // Audio profiles
  if (device.hasAudioSupport()) {
    std::string profiles;
    if (device.supportsA2DP)
      profiles += "A2DP ";
    if (device.supportsHSP)
      profiles += "HSP ";
    if (device.supportsHFP)
      profiles += "HFP ";

    profileLabel = Gtk::make_managed<Gtk::Label>(profiles);
    profileLabel->set_halign(Gtk::Align::START);
    profileLabel->add_css_class("device-profiles");
    profileLabel->add_css_class("caption");
    infoBox->append(*profileLabel);
  }

  append(*infoBox);

  // Action button
  actionButton = Gtk::make_managed<Gtk::Button>();
  actionButton->set_valign(Gtk::Align::CENTER);

  if (device.isConnected) {
    actionButton->set_label("Disconnect");
    actionButton->add_css_class("destructive-action");
  } else {
    actionButton->set_label("Connect");
    actionButton->add_css_class("suggested-action");
  }

  actionButton->signal_clicked().connect(
      sigc::mem_fun(*this, &DeviceRow::onActionClicked));

  append(*actionButton);
}

void DeviceRow::onActionClicked() {
  if (isConnected) {
    m_signal_disconnect.emit(macAddress);
  } else {
    m_signal_connect.emit(macAddress);
  }
}

sigc::signal<void(const std::string &)> DeviceRow::signal_connect_clicked() {
  return m_signal_connect;
}

sigc::signal<void(const std::string &)> DeviceRow::signal_disconnect_clicked() {
  return m_signal_disconnect;
}

void DeviceRow::updateStatus(bool connected, bool paired) {
  isConnected = connected;

  remove_css_class("connected");
  remove_css_class("paired");

  if (connected) {
    statusIcon->set_from_icon_name("bluetooth-active-symbolic");
    add_css_class("connected");
    actionButton->set_label("Disconnect");
    actionButton->remove_css_class("suggested-action");
    actionButton->add_css_class("destructive-action");
  } else if (paired) {
    statusIcon->set_from_icon_name("bluetooth-symbolic");
    add_css_class("paired");
    actionButton->set_label("Connect");
    actionButton->remove_css_class("destructive-action");
    actionButton->add_css_class("suggested-action");
  } else {
    statusIcon->set_from_icon_name("bluetooth-disabled-symbolic");
    actionButton->set_label("Connect");
    actionButton->remove_css_class("destructive-action");
    actionButton->add_css_class("suggested-action");
  }
}

// ============================================================================
// MainWindow Implementation
// ============================================================================

MainWindow::MainWindow() {
  set_title("ToothDroid v2.0");
  set_default_size(450, 600);

  // Initialize Bluetooth manager
  try {
    btManager = std::make_unique<BluetoothManager>();
    btManager->unblockAdapter();
    btManager->powerOn();
  } catch (const std::exception &e) {
    std::cerr << "Failed to initialize Bluetooth: " << e.what() << std::endl;
  }

  // Create header bar
  headerBar = Gtk::make_managed<Gtk::HeaderBar>();
  set_titlebar(*headerBar);

  // Scan button with spinner
  auto *scanBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 6);

  loadingSpinner = Gtk::make_managed<Gtk::Spinner>();
  loadingSpinner->set_visible(false);
  scanBox->append(*loadingSpinner);

  scanButton = Gtk::make_managed<Gtk::Button>("Scan");
  scanButton->set_icon_name("view-refresh-symbolic");
  scanButton->add_css_class("suggested-action");
  scanButton->signal_clicked().connect(
      sigc::mem_fun(*this, &MainWindow::onScanClicked));
  scanBox->append(*scanButton);

  headerBar->pack_start(*scanBox);

  // Settings button
  settingsButton = Gtk::make_managed<Gtk::Button>();
  settingsButton->set_icon_name("preferences-system-symbolic");
  settingsButton->add_css_class("settings-button");
  settingsButton->signal_clicked().connect(
      sigc::mem_fun(*this, &MainWindow::onSettingsClicked));
  headerBar->pack_end(*settingsButton);

  // Main content box
  auto *mainBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 0);
  set_child(*mainBox);

  // Progress bar (hidden by default)
  scanProgress = Gtk::make_managed<Gtk::ProgressBar>();
  scanProgress->set_visible(false);
  scanProgress->add_css_class("osd");
  mainBox->append(*scanProgress);

  // Main stack (for switching between states)
  mainStack = Gtk::make_managed<Gtk::Stack>();
  mainStack->set_transition_type(Gtk::StackTransitionType::CROSSFADE);
  mainStack->set_vexpand(true);
  mainBox->append(*mainStack);

  // Empty state
  emptyState = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 12);
  emptyState->set_valign(Gtk::Align::CENTER);
  emptyState->set_halign(Gtk::Align::CENTER);

  auto *emptyIcon = Gtk::make_managed<Gtk::Image>();
  emptyIcon->set_from_icon_name("bluetooth-symbolic");
  emptyIcon->set_pixel_size(96);
  emptyIcon->add_css_class("dim-label");
  emptyState->append(*emptyIcon);

  auto *emptyLabel = Gtk::make_managed<Gtk::Label>("No Devices Found");
  emptyLabel->add_css_class("title-2");
  emptyState->append(*emptyLabel);

  auto *emptyDesc = Gtk::make_managed<Gtk::Label>(
      "Click Scan to search for nearby Bluetooth devices");
  emptyDesc->add_css_class("dim-label");
  emptyState->append(*emptyDesc);

  mainStack->add(*emptyState, "empty");

  // Device list in a scrolled window
  scrolledDevices = Gtk::make_managed<Gtk::ScrolledWindow>();
  scrolledDevices->set_policy(Gtk::PolicyType::NEVER,
                              Gtk::PolicyType::AUTOMATIC);

  deviceList = Gtk::make_managed<Gtk::ListBox>();
  deviceList->set_selection_mode(Gtk::SelectionMode::NONE);
  deviceList->add_css_class("boxed-list");
  deviceList->set_margin(12);
  scrolledDevices->set_child(*deviceList);

  mainStack->add(*scrolledDevices, "devices");

  // Status bar
  auto *statusBar =
      Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 6);
  statusBar->set_margin(6);
  statusBar->add_css_class("toolbar");

  statusLabel = Gtk::make_managed<Gtk::Label>("Ready");
  statusLabel->set_halign(Gtk::Align::START);
  statusLabel->set_hexpand(true);
  statusLabel->add_css_class("dim-label");
  statusBar->append(*statusLabel);

  mainBox->append(*statusBar);

  // Show empty state initially
  mainStack->set_visible_child(*emptyState);

  // Setup dispatcher for thread-safe UI updates
  dispatcher.connect(sigc::mem_fun(*this, &MainWindow::onDispatcherSignal));

  // Auto-scan on startup after a short delay
  Glib::signal_timeout().connect_once(
      [this]() { startScan(); }, 500); // 500ms delay to let window render first
}

MainWindow::~MainWindow() {
  if (scanThread.joinable()) {
    scanning = false;
    scanThread.join();
  }
}

MainWindow *MainWindow::create() { return new MainWindow(); }

void MainWindow::onScanClicked() {
  if (scanning) {
    stopScan();
  } else {
    startScan();
  }
}

void MainWindow::startScan() {
  setScanning(true);
  statusLabel->set_text("Scanning for devices...");

  // Start scan in background thread
  scanThread = std::thread([this]() {
    try {
      auto devices = btManager->scanDevices(8);

      {
        std::lock_guard<std::mutex> lock(devicesMutex);
        pendingDevices = devices;
      }

      // Signal main thread to update UI
      dispatcher.emit();

    } catch (const std::exception &e) {
      std::cerr << "Scan error: " << e.what() << std::endl;
    }

    scanning = false;
  });

  scanThread.detach();
}

void MainWindow::stopScan() {
  scanning = false;
  setScanning(false);
  statusLabel->set_text("Scan stopped");
}

void MainWindow::setScanning(bool state) {
  scanning = state;

  if (state) {
    scanButton->set_label("Stop");
    scanButton->set_icon_name("process-stop-symbolic");
    scanButton->remove_css_class("suggested-action");
    loadingSpinner->set_visible(true);
    loadingSpinner->start();
    scanProgress->set_visible(true);
    scanProgress->pulse();
  } else {
    scanButton->set_label("Scan");
    scanButton->set_icon_name("view-refresh-symbolic");
    scanButton->add_css_class("suggested-action");
    loadingSpinner->stop();
    loadingSpinner->set_visible(false);
    scanProgress->set_visible(false);
  }
}

void MainWindow::onDispatcherSignal() {
  std::vector<BluetoothDevice> devices;
  {
    std::lock_guard<std::mutex> lock(devicesMutex);
    devices = pendingDevices;
  }

  updateDeviceList(devices);
  setScanning(false);
}

void MainWindow::updateDeviceList(const std::vector<BluetoothDevice> &devices) {
  // Clear existing items
  while (auto *child = deviceList->get_first_child()) {
    deviceList->remove(*child);
  }

  if (devices.empty()) {
    mainStack->set_visible_child(*emptyState);
    statusLabel->set_text("No devices found");
    return;
  }

  // Add device rows
  for (const auto &device : devices) {
    auto *row = Gtk::make_managed<DeviceRow>(device);

    row->signal_connect_clicked().connect(
        sigc::mem_fun(*this, &MainWindow::onDeviceConnect));
    row->signal_disconnect_clicked().connect(
        sigc::mem_fun(*this, &MainWindow::onDeviceDisconnect));

    deviceList->append(*row);
  }

  mainStack->set_visible_child(*scrolledDevices);
  statusLabel->set_text("Found " + std::to_string(devices.size()) +
                        " device(s)");
}

void MainWindow::onDeviceConnect(const std::string &mac) {
  statusLabel->set_text("Connecting to " + mac + "...");

  // Run connection in background
  std::thread([this, mac]() {
    bool success = btManager->connectDevice(mac);

    Glib::signal_idle().connect_once([this, mac, success]() {
      if (success) {
        statusLabel->set_text("Connected to " + mac);
        showNotification("Connected successfully!");

        // Update row - deviceList contains ListBoxRows which wrap our DeviceRow
        for (int i = 0;; i++) {
          auto *listBoxRow = deviceList->get_row_at_index(i);
          if (!listBoxRow)
            break;

          auto *child = listBoxRow->get_child();
          auto *row = dynamic_cast<DeviceRow *>(child);
          if (row && row->getMacAddress() == mac) {
            row->updateStatus(true, true);
            break;
          }
        }
      } else {
        statusLabel->set_text("Connection failed");
        showNotification("Connection failed", true);
      }
    });
  }).detach();
}

void MainWindow::onDeviceDisconnect(const std::string &mac) {
  statusLabel->set_text("Disconnecting...");

  std::thread([this, mac]() {
    btManager->disconnectDevice(mac);

    Glib::signal_idle().connect_once([this, mac]() {
      statusLabel->set_text("Disconnected");

      // Update row - deviceList contains ListBoxRows which wrap our DeviceRow
      for (int i = 0;; i++) {
        auto *listBoxRow = deviceList->get_row_at_index(i);
        if (!listBoxRow)
          break;

        auto *child = listBoxRow->get_child();
        auto *row = dynamic_cast<DeviceRow *>(child);
        if (row && row->getMacAddress() == mac) {
          row->updateStatus(false, true);
          break;
        }
      }
    });
  }).detach();
}

void MainWindow::onSettingsClicked() {
  // Show adapter info dialog
  auto *dialog = Gtk::make_managed<Gtk::MessageDialog>(
      *this, "Bluetooth Adapter", false, Gtk::MessageType::INFO,
      Gtk::ButtonsType::OK, true);

  std::string info = btManager->getAdapterInfo();
  dialog->set_secondary_text(info);
  dialog->set_modal(true);
  dialog->signal_response().connect([dialog](int) { dialog->hide(); });
  dialog->show();
}

void MainWindow::showNotification(const std::string &message, bool isError) {
  // Could integrate with libnotify here
  std::cout << (isError ? "[ERROR] " : "[INFO] ") << message << std::endl;
}

// ============================================================================
// ToothDroidApp Implementation
// ============================================================================

ToothDroidApp::ToothDroidApp()
    : Gtk::Application("org.toothdroid.app",
                       Gio::Application::Flags::DEFAULT_FLAGS) {}

Glib::RefPtr<ToothDroidApp> ToothDroidApp::create() {
  return Glib::make_refptr_for_instance<ToothDroidApp>(new ToothDroidApp());
}

void ToothDroidApp::on_startup() {
  Gtk::Application::on_startup();
  applyCSS();
}

void ToothDroidApp::on_activate() { createWindow(); }

void ToothDroidApp::createWindow() {
  if (!mainWindow) {
    mainWindow = new MainWindow();
    add_window(*mainWindow);
  }

  mainWindow->present();
}

void ToothDroidApp::applyCSS() {
  auto css = Gtk::CssProvider::create();

  const std::string cssData = R"CSS(
        /* ═══════════════════════════════════════════════════════════════
           TOOTHDROID - MACOS INSPIRED THEME (Dark Mode)
           Polished • Unified • Elegant
           ═══════════════════════════════════════════════════════════════ */
        
        /* Global Reset & Typography */
        * {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
        }

        /* Main Window Background */
        window, .background {
            background-color: #1e1e1e;
            color: #ffffff;
        }
        
        /* HeaderBar - Unified Toolbar Style - FORCE DARK */
        headerbar, .titlebar, window > headerbar {
            background-color: #2c2c2c !important;
            background-image: none !important;
            border-bottom: 1px solid #000000 !important;
            min-height: 52px;
            box-shadow: none !important;
            padding: 0 12px;
            color: #ffffff !important;
        }
        
        headerbar title, headerbar label {
            font-weight: 600;
            font-size: 15px;
            color: #ffffff !important;
            opacity: 0.9;
        }
        
        /* EXCLUDE Window Controls from generic styling */
        windowcontrols button {
            background-color: transparent !important;
            box-shadow: none !important;
            border: none !important;
            margin: 0;
            padding: 0;
            min-height: 24px;
            min-width: 24px;
        }

        /* Generic Header Button - Reset */
        headerbar button {
            background-color: transparent !important;
            background-image: none !important;
            border: none !important;
            box-shadow: none !important;
            padding: 6px 12px;
            margin: 4px;
            color: #ffffff !important;
        }

        headerbar button:hover {
            background-color: rgba(255, 255, 255, 0.1) !important;
        }

        /* Suggested Action (Scan Button) - System Blue */
        headerbar button.suggested-action {
            background-color: #007AFF !important;
            color: white !important;
            border-radius: 6px;
            border: none !important;
            padding: 6px 12px;
            font-weight: 500;
            box-shadow: 0 1px 2px rgba(0, 0, 0, 0.2) !important;
            margin: 6px 0;
        }
        
        headerbar button.suggested-action:hover {
            background-color: #0062cc !important;
        }
        
        headerbar button.suggested-action:active {
            background-color: #0051a8 !important;
            transform: scale(0.98);
        }
        
        /* Settings Button Specifics */
        .settings-button {
            min-width: 32px;
            min-height: 32px;
            padding: 0 !important;
            margin: 6px;
            border-radius: 8px;
            background-color: transparent !important;
            color: #cccccc !important;
            border: 1px solid rgba(255, 255, 255, 0.2) !important;
            opacity: 1 !important;
        }
        
        .settings-button:hover {
            background-color: rgba(255, 255, 255, 0.1) !important;
            color: #ffffff !important;
            border-color: rgba(255, 255, 255, 0.5) !important;
        }

        /* Device List - Inset Grouped Style */
        .boxed-list {
            background-color: transparent;
            margin: 10px;
        }

        /* Device Row (Card) */
        .device-row {
            background-color: #333333; /* Slightly lighter card */
            border-radius: 10px;
            margin-bottom: 8px;
            padding: 14px 16px;
            border: 1px solid rgba(0, 0, 0, 0.2);
            box-shadow: 0 2px 8px rgba(0, 0, 0, 0.15);
            transition: transform 0.2s ease, background-color 0.2s ease;
        }

        .device-row:hover {
            background-color: #3d3d3d;
            transform: translateY(-1px);
        }
        
        /* States: Connected */
        .device-row.connected {
            background-color: #1c2e24;
            border: 1px solid #30d158;
        }
        
        .device-row.connected .device-name {
            color: #30d158;
        }
        
        /* States: Paired */
        .device-row.paired {
            border: 1px solid #0a84ff;
        }
        
        .device-row.paired .device-name {
            color: #0a84ff;
        }

        /* Typography */
        .device-name {
            font-size: 15px;
            font-weight: 600;
            letter-spacing: -0.2px;
            color: #ffffff;
            margin-bottom: 2px;
        }

        .device-mac {
            font-family: "SF Mono", "Menlo", monospace;
            font-size: 11px;
            color: #98989d;
        }
        
        .device-profiles {
            background-color: rgba(255, 255, 255, 0.1);
            border-radius: 4px;
            padding: 2px 6px;
            font-size: 10px;
            color: #e5e5ea;
            font-weight: 500;
            margin-left: 8px;
        }

        /* Action Buttons inside Row */
        .device-row button {
             border: none;
             box-shadow: none;
        }

        .device-row button.suggested-action {
            background-color: #30d158;
            color: white;
            border-radius: 16px;
            padding: 4px 16px;
            font-size: 12px;
            font-weight: 600;
            min-height: 28px;
        }
        
        .device-row button.suggested-action:hover {
            background-color: #28cd41;
        }
        
        .device-row button.destructive-action {
            background-color: rgba(255, 69, 58, 0.15);
            color: #ff453a;
            border-radius: 16px;
            padding: 4px 16px;
            font-size: 12px;
            font-weight: 600;
            border: 1px solid rgba(255, 69, 58, 0.3);
            min-height: 28px;
        }
        
        .device-row button.destructive-action:hover {
            background-color: rgba(255, 69, 58, 0.25);
        }

        /* Status Bar (Toolbar) - Fix white background issue */
        .toolbar {
            background-color: #1e1e1e; /* Match window background */
            border-top: 1px solid #000000;
            color: #8e8e93;
            padding: 8px;
        }
        
        .dim-label {
            color: #8e8e93;
            font-weight: 400;
        }
        
        .title-2 {
            font-size: 22px;
            font-weight: 700;
            color: white;
            margin-bottom: 8px;
        }
        
        /* Dialogs */
        dialog {
            background-color: #2c2c2c;
            border-radius: 12px;
            color: #ffffff;
            border: 1px solid #000000;
        }
        
        message-dialog {
             background-color: #2c2c2c;
             color: #ffffff;
        }
        
        /* Scrollbar */
        scrollbar {
            background-color: transparent;
        }
        
        scrollbar slider {
            min-width: 6px;
            border-radius: 10px;
            background-color: rgba(255, 255, 255, 0.2);
            border: 2px solid transparent;
            background-clip: padding-box;
        }
        
        scrollbar slider:hover {
            background-color: rgba(255, 255, 255, 0.4);
        }
        
        /* ProgressBar */
        progressbar progress {
            background-color: #007AFF;
            border-radius: 2px;
        }
        
        progressbar trough {
            background-color: rgba(255, 255, 255, 0.1);
            border-radius: 2px;
            min-height: 3px;
        }
    )CSS";

  css->load_from_data(cssData);

  Gtk::StyleContext::add_provider_for_display(
      Gdk::Display::get_default(), css, GTK_STYLE_PROVIDER_PRIORITY_USER + 100);
}

} // namespace GUI
} // namespace ToothDroid
