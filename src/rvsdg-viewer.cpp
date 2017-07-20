#include <QApplication>

#include "mainwindow.h"

int main(int argc, char *argv[]) {
  Q_INIT_RESOURCE(application);

  QApplication app(argc, argv);
  app.setApplicationName("RVSDG Viewer");

  MainWindow *mainWin;

  if(QCoreApplication::arguments().size() > 1) {
    mainWin = new MainWindow(QCoreApplication::arguments().at(1));
  } else {
    mainWin = new MainWindow();
  }
  mainWin->show();
  return app.exec();
}
