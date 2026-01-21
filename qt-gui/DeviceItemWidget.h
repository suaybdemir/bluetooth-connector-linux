#ifndef DEVICEITEMWIDGET_H
#define DEVICEITEMWIDGET_H

#include "../include/BluetoothDevice.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

namespace ToothDroid {
namespace GUI {

class DeviceItemWidget : public QWidget {
  Q_OBJECT

public:
  explicit DeviceItemWidget(const BluetoothDevice &device,
                            QWidget *parent = nullptr);
  void updateStatus(bool connected, bool paired);
  QString getMacAddress() const {
    return QString::fromStdString(m_device.macAddress);
  }

signals:
  void connectClicked(const QString &mac);
  void disconnectClicked(const QString &mac);

private:
  BluetoothDevice m_device;
  QLabel *m_iconLabel;
  QLabel *m_nameLabel;
  QLabel *m_macLabel;
  QLabel *m_profileLabel;
  QPushButton *m_actionButton;
  bool m_isConnected;

  void setupUi();
};

} // namespace GUI
} // namespace ToothDroid

#endif // DEVICEITEMWIDGET_H
