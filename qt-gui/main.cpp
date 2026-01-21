#include "MainWindow.h"
#include <QApplication>
#include <QFile>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  // Load stylesheet
  QFile styleFile("qt-gui/style.qss");
  if (styleFile.open(QFile::ReadOnly)) {
    QString style = QLatin1String(styleFile.readAll());
    app.setStyleSheet(style);
  }

  ToothDroid::GUI::MainWindow window;
  window.show();

  return app.exec();
}
