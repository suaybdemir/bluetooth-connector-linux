#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../include/BluetoothManager.h"
#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QMenu>
#include <QMouseEvent>
#include <QPoint>
#include <QProgressBar>
#include <QPushButton>
#include <QThread>
#include <QTimer>
#include <memory>
#include <mutex>

namespace ToothDroid {
namespace GUI {

// Worker thread for scanning to keep UI responsive
class ScanWorker : public QObject {
  Q_OBJECT
public:
  explicit ScanWorker(BluetoothManager *manager) : m_manager(manager) {}

public slots:
  void process() {
    try {
      // Scan for 8 seconds
      auto devices = m_manager->scanDevices(8);
      emit finished(devices);
    } catch (const std::exception &e) {
      emit error(QString::fromStdString(e.what()));
    }
  }

signals:
  void finished(std::vector<BluetoothDevice> devices);
  void error(QString err);

private:
  BluetoothManager *m_manager;
};

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  // Public for helper functions
  void log(const QString &msg);

protected:
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;

private slots:
  void startScan();
  void onScanFinished(const std::vector<BluetoothDevice> &devices);
  void onScanError(const QString &err);
  void connectDevice(const QString &mac);
  void disconnectDevice(const QString &mac);
  void removeDevice(const QString &mac);
  void pairDevice(const QString &mac);
  void showAboutDialog();

  // New Actions
  void trustDevice(const QString &mac);
  void blockDevice(const QString &mac);
  void showDeviceInfo(const QString &mac);

  // Window controls
  void minimizeWindow();
  void maximizeWindow();
  void closeWindow();

  // Context Menu
  void onContextMenuRequested(const QPoint &pos);

private:
  void setupUi();
  void updateDeviceList(const std::vector<BluetoothDevice> &devices);
  void setScanning(bool scanning);

  // Window dragging
  QPoint m_dragPosition;

  // UI Components
  QListWidget *m_deviceList;
  QPushButton *m_scanButton;
  QProgressBar *m_progressBar;
  QLabel *m_statusLabel;
  QWidget *m_emptyState;

  // Bluetooth Logic
  std::unique_ptr<BluetoothManager> m_manager;
  QThread *m_scanThread = nullptr;
  ScanWorker *m_scanWorker = nullptr;
  bool m_isScanning = false;
};

} // namespace GUI
} // namespace ToothDroid

#endif // MAINWINDOW_H
