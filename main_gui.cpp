/**
 * @file main_gui.cpp
 * @brief ToothDroid GTK4 GUI Entry Point
 */

#include "gui/MainWindow.h"

int main(int argc, char *argv[]) {
  auto app = ToothDroid::GUI::ToothDroidApp::create();
  return app->run(argc, argv);
}
