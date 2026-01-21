#include "MainWindow.h"
#include "DeviceItemWidget.h"
#include <QDir>
#include <QFile>
#include <QHBoxLayout>
#include <QMenu>
#include <QMessageBox>
#include <QPointer>
#include <QStyle>
#include <QVBoxLayout>
#include <thread>

namespace ToothDroid {
namespace GUI {

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setWindowTitle("ToothDroid v2.2");
  resize(420, 680);

  // Remove system title bar and setup translucency
  setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
  setAttribute(Qt::WA_TranslucentBackground);

  // Initialize Bluetooth
  try {
    m_manager = std::make_unique<BluetoothManager>();
    m_manager->unblockAdapter();
    m_manager->powerOn();
  } catch (const std::exception &e) {
    QMessageBox::critical(
        this, "Bluetooth Error",
        QString("Failed to initialize Bluetooth:\n%1").arg(e.what()));
  }

  setupUi();

  // Auto scan on startup
  QTimer::singleShot(200, this, [this]() { startScan(); });
}

MainWindow::~MainWindow() {
  if (m_scanThread && m_scanThread->isRunning()) {
    m_scanThread->quit();
    m_scanThread->wait();
  }
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton && event->pos().y() < 60) {
    m_dragPosition =
        event->globalPosition().toPoint() - frameGeometry().topLeft();
    event->accept();
  }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
  if (event->buttons() & Qt::LeftButton && event->pos().y() < 60) {
    move(event->globalPosition().toPoint() - m_dragPosition);
    event->accept();
  }
}

void MainWindow::minimizeWindow() { showMinimized(); }
void MainWindow::maximizeWindow() {
  isMaximized() ? showNormal() : showMaximized();
}
void MainWindow::closeWindow() { close(); }

void MainWindow::setupUi() {
  auto *centralWidget = new QWidget(this);
  centralWidget->setObjectName("centralWidget");
  centralWidget->setStyleSheet(
      "#centralWidget { background-color: #1e1e1e; border: 1px solid #333333; "
      "border-radius: 12px; }");
  setCentralWidget(centralWidget);

  auto *mainLayout = new QVBoxLayout(centralWidget);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  // --- Header Bar ---
  auto *headerWidget = new QWidget(this);
  headerWidget->setStyleSheet(
      "background-color: #2c2c2c; border-bottom: 1px solid #111; "
      "border-top-left-radius: 11px; border-top-right-radius: 11px;");
  headerWidget->setFixedHeight(52);

  auto *headerLayout = new QHBoxLayout(headerWidget);
  headerLayout->setContentsMargins(16, 0, 16, 0);
  headerLayout->setSpacing(12);

  // Mac-style Window Controls
  auto *controlsLayout = new QHBoxLayout();
  controlsLayout->setSpacing(8);

  // Close (Red)
  auto *closeBtn = new QPushButton(this);
  closeBtn->setFixedSize(12, 12);
  closeBtn->setStyleSheet(
      "QPushButton { background-color: #ff5f56; border-radius: 6px; border: "
      "none; } QPushButton:hover { background-color: #ff3b30; }");
  connect(closeBtn, &QPushButton::clicked, this, &MainWindow::closeWindow);
  controlsLayout->addWidget(closeBtn);

  // Minimize (Yellow)
  auto *minBtn = new QPushButton(this);
  minBtn->setFixedSize(12, 12);
  minBtn->setStyleSheet(
      "QPushButton { background-color: #ffbd2e; border-radius: 6px; border: "
      "none; } QPushButton:hover { background-color: #ffad1f; }");
  connect(minBtn, &QPushButton::clicked, this, &MainWindow::minimizeWindow);
  controlsLayout->addWidget(minBtn);

  // Maximize (Green)
  auto *maxBtn = new QPushButton(this);
  maxBtn->setFixedSize(12, 12);
  maxBtn->setStyleSheet(
      "QPushButton { background-color: #27c93f; border-radius: 6px; border: "
      "none; } QPushButton:hover { background-color: #2ebd41; }");
  connect(maxBtn, &QPushButton::clicked, this, &MainWindow::maximizeWindow);
  controlsLayout->addWidget(maxBtn);

  headerLayout->addLayout(controlsLayout);
  headerLayout->addStretch();

  // App Title
  auto *titleLabel = new QLabel("ToothDroid", this);
  titleLabel->setStyleSheet("font-weight: 600; color: #ddd; font-size: 14px;");
  headerLayout->addWidget(titleLabel);
  headerLayout->addStretch();

  // Settings
  auto *settingsButton = new QPushButton("⚙️", this);
  settingsButton->setFixedSize(32, 32);
  settingsButton->setCursor(Qt::PointingHandCursor);
  settingsButton->setStyleSheet(
      "QPushButton { background: transparent; border: none; font-size: 16px; "
      "border-radius: 4px; } QPushButton:hover { background: "
      "rgba(255,255,255,0.1); }");
  connect(settingsButton, &QPushButton::clicked, this,
          &MainWindow::showAboutDialog);
  headerLayout->addWidget(settingsButton);

  mainLayout->addWidget(headerWidget);

  // Progress
  m_progressBar = new QProgressBar(this);
  m_progressBar->setRange(0, 0);
  m_progressBar->setVisible(false);
  m_progressBar->setFixedHeight(2);
  m_progressBar->setStyleSheet(
      "QProgressBar { background: transparent; border: none; } "
      "QProgressBar::chunk { background-color: #007AFF; }");
  mainLayout->addWidget(m_progressBar);

  // Main Content
  auto *contentStack = new QWidget(this);
  auto *stackLayout = new QVBoxLayout(contentStack);

  // Empty State
  m_emptyState = new QWidget(this);
  auto *emptyLayout = new QVBoxLayout(m_emptyState);
  emptyLayout->setAlignment(Qt::AlignCenter);
  auto *emptyIcon = new QLabel("📡", this);
  emptyIcon->setStyleSheet(
      "font-size: 64px; margin-bottom: 20px; color: #555;");
  auto *emptyTitle = new QLabel("No Devices Found", this);
  emptyTitle->setStyleSheet(
      "font-size: 20px; font-weight: bold; color: #eee; margin-bottom: 8px;");
  auto *emptyDesc = new QLabel("Click Scan below to search", this);
  emptyDesc->setStyleSheet("color: #888;");

  emptyLayout->addWidget(emptyIcon, 0, Qt::AlignCenter);
  emptyLayout->addWidget(emptyTitle, 0, Qt::AlignCenter);
  emptyLayout->addWidget(emptyDesc, 0, Qt::AlignCenter);

  // Device List
  m_deviceList = new QListWidget(this);
  m_deviceList->setVisible(false);
  m_deviceList->setContextMenuPolicy(Qt::CustomContextMenu);
  m_deviceList->setStyleSheet(
      "QListWidget { background: transparent; border: none; outline: none; }");
  m_deviceList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  connect(m_deviceList, &QListWidget::customContextMenuRequested, this,
          &MainWindow::onContextMenuRequested);

  stackLayout->addWidget(m_emptyState);
  stackLayout->addWidget(m_deviceList);

  mainLayout->addWidget(contentStack, 1);

  // Footer
  auto *statusBar = new QWidget(this);
  statusBar->setStyleSheet(
      "background-color: #1e1e1e; border-top: 1px solid #333; padding: 10px; "
      "border-bottom-left-radius: 12px; border-bottom-right-radius: 12px;");
  auto *statusLayout = new QHBoxLayout(statusBar);
  statusLayout->setContentsMargins(12, 0, 12, 0);

  m_statusLabel = new QLabel("Ready", this);
  m_statusLabel->setStyleSheet("color: #666; font-size: 11px;");
  statusLayout->addWidget(m_statusLabel);

  statusLayout->addStretch();

  // Scan Button
  m_scanButton = new QPushButton("Scan", this);
  m_scanButton->setCursor(Qt::PointingHandCursor);
  m_scanButton->setFixedWidth(120); // Wider for better touch area
  m_scanButton->setStyleSheet(
      "QPushButton { background-color: #333; color: white; border-radius: 6px; "
      "padding: 8px; font-weight: 600; border: 1px solid #444; }"
      "QPushButton:hover { background-color: #444; border-color: #555; }"
      "QPushButton:pressed { background-color: #222; transform: "
      "translateY(1px); }"
      "QPushButton:disabled { color: #555; border-color: #2a2a2a; }");
  connect(m_scanButton, &QPushButton::clicked, this, &MainWindow::startScan);
  statusLayout->addWidget(m_scanButton);

  mainLayout->addWidget(statusBar);
}

void MainWindow::startScan() {
  if (m_isScanning)
    return;

  setScanning(true);
  m_statusLabel->setText("Scanning...");

  m_scanThread = new QThread;
  m_scanWorker = new ScanWorker(m_manager.get());
  m_scanWorker->moveToThread(m_scanThread);

  connect(m_scanThread, &QThread::started, m_scanWorker, &ScanWorker::process);
  connect(m_scanWorker, &ScanWorker::finished, this,
          &MainWindow::onScanFinished);
  connect(m_scanWorker, &ScanWorker::finished, m_scanThread, &QThread::quit);
  connect(m_scanWorker, &ScanWorker::finished, m_scanWorker,
          &ScanWorker::deleteLater);
  connect(m_scanThread, &QThread::finished, m_scanThread,
          &QThread::deleteLater);

  m_scanThread->start();
}

void MainWindow::onScanFinished(const std::vector<BluetoothDevice> &devices) {
  setScanning(false);
  updateDeviceList(devices);
}

void MainWindow::onScanError(const QString &err) {
  setScanning(false);
  m_statusLabel->setText("Error: " + err);
}

void MainWindow::setScanning(bool scanning) {
  m_isScanning = scanning;
  m_progressBar->setVisible(scanning);
  m_scanButton->setEnabled(!scanning);
  m_scanButton->setText(scanning ? "Scanning..." : "Scan");
}

void MainWindow::updateDeviceList(const std::vector<BluetoothDevice> &devices) {
  m_deviceList->clear();

  if (devices.empty()) {
    m_emptyState->setVisible(true);
    m_deviceList->setVisible(false);
    m_statusLabel->setText("No devices found");
    return;
  }

  m_emptyState->setVisible(false);
  m_deviceList->setVisible(true);

  for (const auto &device : devices) {
    auto *item = new QListWidgetItem(m_deviceList);
    auto *widget = new DeviceItemWidget(device);

    item->setSizeHint(QSize(0, 72));

    connect(widget, &DeviceItemWidget::connectClicked, this,
            &MainWindow::connectDevice);
    connect(widget, &DeviceItemWidget::disconnectClicked, this,
            &MainWindow::disconnectDevice);

    m_deviceList->addItem(item);
    m_deviceList->setItemWidget(item, widget);
  }

  m_statusLabel->setText(QString("Found %1 devices").arg(devices.size()));
}

// Thread-safe helper
template <typename Func>
void runInThread(MainWindow *self, const QString &mac, Func func) {
  QPointer<MainWindow> safeSelf(self);
  std::thread([safeSelf, mac, func]() {
    if (!safeSelf)
      return;
    try {
      bool result = func();
      // Check self again before posting event
      // Note: invokeMethod takes the object as context, so if self dies, it
      // *should* remain safe or just fail to queue But QPointer check inside
      // the thread body before lambda creation is good. Be careful capturing
      // safeSelf in lambda, it's a QPointer.
      QMetaObject::invokeMethod(safeSelf, [safeSelf, result, mac]() {
        if (safeSelf) {
          if (result)
            safeSelf->log("Action success: " + mac);
          else
            safeSelf->log("Action failed: " + mac);
        }
      });
    } catch (...) {
    }
  }).detach();
}

void MainWindow::log(const QString &msg) { m_statusLabel->setText(msg); }

void MainWindow::connectDevice(const QString &mac) {
  m_statusLabel->setText("Connecting " + mac + "...");
  runInThread(this, mac, [this, mac]() {
    return m_manager->connectDevice(mac.toStdString());
  });
}

void MainWindow::disconnectDevice(const QString &mac) {
  m_statusLabel->setText("Disconnecting " + mac + "...");
  runInThread(this, mac, [this, mac]() {
    return m_manager->disconnectDevice(mac.toStdString());
  });
}

void MainWindow::removeDevice(const QString &mac) {
  m_manager->removeDevice(mac.toStdString());
  startScan();
}

void MainWindow::pairDevice(const QString &mac) {
  m_statusLabel->setText("Pairing " + mac + "...");
  runInThread(this, mac, [this, mac]() {
    return m_manager->pairDevice(mac.toStdString());
  });
}

void MainWindow::trustDevice(const QString &mac) {
  m_statusLabel->setText("Trusting " + mac + "...");
  runInThread(this, mac, [this, mac]() {
    return m_manager->trustDevice(mac.toStdString());
  });
}

void MainWindow::blockDevice(const QString &mac) {
  m_statusLabel->setText("Blocking " + mac + "...");
  runInThread(this, mac, [this, mac]() {
    return m_manager->blockDevice(mac.toStdString());
  });
}

void MainWindow::showDeviceInfo(const QString &mac) {
  // In a real app this would query more details
  QMessageBox::information(this, "Device Info",
                           "MAC Address: " + mac +
                               "\n\nAdvanced info requires device connection.");
}

void MainWindow::onContextMenuRequested(const QPoint &pos) {
  auto *item = m_deviceList->itemAt(pos);
  if (!item)
    return;

  auto *widget =
      qobject_cast<DeviceItemWidget *>(m_deviceList->itemWidget(item));
  if (!widget)
    return;

  QString mac = widget->getMacAddress();
  QMenu contextMenu(this);
  contextMenu.setStyleSheet(
      "QMenu { background-color: #2a2a2a; color: #eee; border: 1px solid #444; "
      "border-radius: 6px; padding: 4px; }"
      "QMenu::item { padding: 6px 24px; border-radius: 4px; margin: 2px; }"
      "QMenu::item:selected { background-color: #007AFF; color: white; }"
      "QMenu::separator { height: 1px; background: #444; margin: 4px 0; }");

  // Actions
  auto *connectAct = contextMenu.addAction("Connect");
  auto *disconnectAct = contextMenu.addAction("Disconnect");
  contextMenu.addSeparator();
  auto *pairAct = contextMenu.addAction("Pair");
  auto *unpairAct = contextMenu.addAction("Unpair / Forget");
  contextMenu.addSeparator();
  auto *trustAct = contextMenu.addAction("Trust");
  auto *blockAct = contextMenu.addAction("Block");
  contextMenu.addSeparator();
  auto *infoAct = contextMenu.addAction("Device Info");

  connect(connectAct, &QAction::triggered,
          [this, mac]() { connectDevice(mac); });
  connect(disconnectAct, &QAction::triggered,
          [this, mac]() { disconnectDevice(mac); });
  connect(pairAct, &QAction::triggered, [this, mac]() { pairDevice(mac); });
  connect(unpairAct, &QAction::triggered, [this, mac]() { removeDevice(mac); });
  connect(trustAct, &QAction::triggered, [this, mac]() { trustDevice(mac); });
  connect(blockAct, &QAction::triggered, [this, mac]() { blockDevice(mac); });
  connect(infoAct, &QAction::triggered, [this, mac]() { showDeviceInfo(mac); });

  contextMenu.exec(m_deviceList->mapToGlobal(pos));
}

void MainWindow::showAboutDialog() {
  QMessageBox::about(
      this, "About ToothDroid",
      "<h3>ToothDroid v2.2</h3><p>Mac-styled Bluetooth Manager</p>"
      "<p>Built with Qt6 (LGPL)</p>");
}

} // namespace GUI
} // namespace ToothDroid
