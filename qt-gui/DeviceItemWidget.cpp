#include "DeviceItemWidget.h"
#include <QIcon>
#include <QStyle>

namespace ToothDroid {
namespace GUI {

DeviceItemWidget::DeviceItemWidget(const BluetoothDevice &device,
                                   QWidget *parent)
    : QWidget(parent), m_device(device), m_isConnected(device.isConnected) {
  setupUi();
  updateStatus(device.isConnected, device.isPaired);
}

void DeviceItemWidget::setupUi() {
  auto *mainLayout = new QHBoxLayout(this);
  mainLayout->setContentsMargins(16, 14, 16, 14);
  mainLayout->setSpacing(12);

  // Status Icon (using emoji/text as placeholder for now, ideally SVGs)
  // In a real app we'd load icons from a resource file
  m_iconLabel = new QLabel(this);
  m_iconLabel->setFixedSize(24, 24);
  m_iconLabel->setAlignment(Qt::AlignCenter);
  mainLayout->addWidget(m_iconLabel);

  // Info Column
  auto *infoLayout = new QVBoxLayout();
  infoLayout->setSpacing(4);

  // Name Row
  auto *nameRow = new QHBoxLayout();
  m_nameLabel =
      new QLabel(QString::fromStdString(m_device.getDisplayName()), this);
  m_nameLabel->setObjectName("deviceName");
  nameRow->addWidget(m_nameLabel);
  nameRow->addStretch();
  infoLayout->addLayout(nameRow);

  // Details Row (MAC + profiles)
  auto *detailsRow = new QHBoxLayout();
  m_macLabel = new QLabel(QString::fromStdString(m_device.macAddress), this);
  m_macLabel->setObjectName("deviceMac");
  detailsRow->addWidget(m_macLabel);

  if (m_device.hasAudioSupport()) {
    QString profiles;
    if (m_device.supportsA2DP)
      profiles += "A2DP ";
    if (m_device.supportsHSP)
      profiles += "HSP ";
    if (m_device.supportsHFP)
      profiles += "HFP ";

    m_profileLabel = new QLabel(profiles.trimmed(), this);
    m_profileLabel->setObjectName("deviceProfile");
    detailsRow->addSpacing(8);
    detailsRow->addWidget(m_profileLabel);
  }

  detailsRow->addStretch();
  infoLayout->addLayout(detailsRow);

  mainLayout->addLayout(infoLayout, 1); // Give info layout all extra space

  // Action Button
  m_actionButton = new QPushButton(this);
  m_actionButton->setCursor(Qt::PointingHandCursor);
  m_actionButton->setFixedWidth(100);

  connect(m_actionButton, &QPushButton::clicked, [this]() {
    if (m_isConnected) {
      emit disconnectClicked(QString::fromStdString(m_device.macAddress));
    } else {
      emit connectClicked(QString::fromStdString(m_device.macAddress));
    }
  });

  mainLayout->addWidget(m_actionButton);
}

void DeviceItemWidget::updateStatus(bool connected, bool paired) {
  m_isConnected = connected;

  // Style update logic - we could use dynamic properties for CSS
  if (connected) {
    m_iconLabel->setText("🟢");                    // Green circle
    m_nameLabel->setStyleSheet("color: #30d158;"); // Green text
    m_actionButton->setText("Disconnect");
    m_actionButton->setProperty("danger", true);
    m_actionButton->setProperty("success", false);
  } else if (paired) {
    m_iconLabel->setText("🔵");                    // Blue circle
    m_nameLabel->setStyleSheet("color: #0a84ff;"); // Blue text
    m_actionButton->setText("Connect");
    m_actionButton->setProperty("danger", false);
    m_actionButton->setProperty("success", true);
  } else {
    m_iconLabel->setText("⚪"); // White/gray circle
    m_nameLabel->setStyleSheet("color: white;");
    m_actionButton->setText("Connect");
    m_actionButton->setProperty("danger", false);
    m_actionButton->setProperty("success", true);
  }

  // Force style refresh
  m_actionButton->style()->unpolish(m_actionButton);
  m_actionButton->style()->polish(m_actionButton);
}

} // namespace GUI
} // namespace ToothDroid
