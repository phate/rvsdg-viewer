#include <QDomDocument>

#include "mainwindow.h"

///////////////////////////////////////////////////////////////////////////////

MainWindow::MainWindow() {
  init();
}

MainWindow::MainWindow(const QString &fileName) {
  init();
  loadFile(fileName);
}

///////////////////////////////////////////////////////////////////////////////

void MainWindow::closeEvent(QCloseEvent *event) {
  event->accept();
}

void MainWindow::open() {
  QString fileName = QFileDialog::getOpenFileName(this);
  if(!fileName.isNull()) loadFile(fileName);
}

void MainWindow::about() {
  QMessageBox::about(this, tr("About RVSDG Viewer"), tr("NTNU"));
}

///////////////////////////////////////////////////////////////////////////////

void MainWindow::regionClicked(const QModelIndex &index) {
  Element *el = static_cast<Element*>(index.internalPointer());
  if(el->isRegion()) {
    scene->drawRegion(static_cast<Region*>(el));
  }
}

void MainWindow::init() {
  rvsdgModel = NULL;

  // central widget
  treeView = new QTreeView();

  connect(treeView, SIGNAL(clicked(QModelIndex)), this, SLOT(regionClicked(QModelIndex)));

  scene = new DiagramScene(this);
  scene->setSceneRect(QRectF(0, 0, 1024, 512));

  graphicsView = new QGraphicsView(scene);
  
  splitter = new QSplitter;
  splitter->addWidget(treeView);
  splitter->addWidget(graphicsView);

  setCentralWidget(splitter);

  showMaximized();

  // actions
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

  // menus
  fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(openAct);
  fileMenu->addSeparator();
  fileMenu->addAction(exitAct);

  menuBar()->addSeparator();

  helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addAction(aboutAct);
  helpMenu->addAction(aboutQtAct);

  // toolbar
  fileToolBar = addToolBar(tr("File"));
  fileToolBar->addAction(openAct);

  // statusbar
  statusBar()->showMessage(tr("Ready"));
}

MainWindow::~MainWindow() {
  if(rvsdgModel) delete rvsdgModel;
}

void MainWindow::loadFile(const QString &fileName) {
  QDomDocument doc;
  QFile file(fileName);
  if(!file.open(QIODevice::ReadOnly)) {
    QMessageBox msgBox;
    msgBox.setText("File not found");
    msgBox.exec();
    return;
  }
  if(!doc.setContent(&file)) {
    QMessageBox msgBox;
    msgBox.setText("Invalid XML file");
    msgBox.exec();
    file.close();
    return;
  }
  file.close();

  if(rvsdgModel) delete rvsdgModel;
  try {
    rvsdgModel = new Model(doc);
  } catch (std::exception &e) {
    QMessageBox msgBox;
    msgBox.setText("Invalid RVSDG file");
    msgBox.exec();
    rvsdgModel = NULL;
    return;
  }

  treeView->setModel(rvsdgModel);
  treeView->setColumnWidth(0,250);
}
