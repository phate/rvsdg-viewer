#include <QtWidgets>

#include "mainwindow.h"
#include "treemodel.h"

MainWindow::MainWindow() {
  QWidget *window = new QWidget;
  QPushButton *button1 = new QPushButton("One");

  QFile file("../default.txt");
  file.open(QIODevice::ReadOnly);
  TreeModel *model = new TreeModel(file.readAll());
  file.close();

  QTreeView *view = new QTreeView();
  view->setModel(model);

  QHBoxLayout *layout = new QHBoxLayout;
  layout->addWidget(view);
  layout->addWidget(button1);

  window->setLayout(layout);
  setCentralWidget(window);

  createActions();
  createMenus();
  createToolBars();
  createStatusBar();
}

void MainWindow::closeEvent(QCloseEvent *event) {
  event->accept();
}

void MainWindow::open() {
  QString fileName = QFileDialog::getOpenFileName(this);
}

void MainWindow::about() {
  QMessageBox::about(this, tr("About RVSDG Viewer"),
                     tr("NTNU"));
}

void MainWindow::createActions() {
  openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
  openAct->setShortcuts(QKeySequence::Open);
  openAct->setStatusTip(tr("Open an existing file"));
  connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

  exitAct = new QAction(tr("E&xit"), this);
  exitAct->setShortcuts(QKeySequence::Quit);
  exitAct->setStatusTip(tr("Exit the application"));
  connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

  aboutAct = new QAction(tr("&About"), this);
  aboutAct->setStatusTip(tr("Show the application's About box"));
  connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

  aboutQtAct = new QAction(tr("About &Qt"), this);
  aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
  connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void MainWindow::createMenus() {
  fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(openAct);
  fileMenu->addSeparator();
  fileMenu->addAction(exitAct);

  menuBar()->addSeparator();

  helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addAction(aboutAct);
  helpMenu->addAction(aboutQtAct);
}

void MainWindow::createToolBars() {
  fileToolBar = addToolBar(tr("File"));
  fileToolBar->addAction(openAct);
}

void MainWindow::createStatusBar() {
  statusBar()->showMessage(tr("Ready"));
}

