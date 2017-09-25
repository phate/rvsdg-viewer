#include <QDomDocument>

#include "mainwindow.h"
#include "diagramview.h"

///////////////////////////////////////////////////////////////////////////////

QColor edgeColors[] = EDGE_COLORS;
QString colorNames[] = COLOR_NAMES;

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
  scene->drawElement(el);
}

void MainWindow::init() {
  rvsdgModel = NULL;

  // central widget
  treeView = new QTreeView();

  connect(treeView, SIGNAL(clicked(QModelIndex)), this, SLOT(regionClicked(QModelIndex)));

  colorBox = new QComboBox();
  for(unsigned i = 0; i < sizeof(edgeColors)/sizeof(edgeColors[0]); i++) {
    colorBox->addItem(colorNames[i]);
  }

  scene = new DiagramScene(colorBox, this);
  scene->setSceneRect(QRectF(0, 0, 1024, 512));

  graphicsView = new DiagramView(scene);
  
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

  zoomInAct = new QAction(QIcon(":/images/zoomin.png"), tr("&ZoomIn..."), this);
  zoomInAct->setStatusTip(tr("Zoom in"));
  connect(zoomInAct, SIGNAL(triggered()), graphicsView, SLOT(zoomInEvent()));

  zoomOutAct = new QAction(QIcon(":/images/zoomout.png"), tr("&ZoomOut..."), this);
  zoomOutAct->setStatusTip(tr("Zoom out"));
  connect(zoomOutAct, SIGNAL(triggered()), graphicsView, SLOT(zoomOutEvent()));

  // clearColorsAct = new QAction(tr("&Clear..."), this);
  // clearColorsAct->setStatusTip(tr("Clear edge colors"));
  // connect(clearColorsAct, SIGNAL(triggered()), this, SLOT(clearColorsEvent()));

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
  fileToolBar->addAction(zoomInAct);
  fileToolBar->addAction(zoomOutAct);
  fileToolBar->addWidget(colorBox);
  //fileToolBar->addAction(clearColorsAct);

  // statusbar
  statusBar()->showMessage(tr("Ready"));
}

MainWindow::~MainWindow() {
  if(rvsdgModel) delete rvsdgModel;
}

void MainWindow::loadFile(const QString &fileName) {
  QFileInfo fi(fileName);
  setWindowTitle("RVSDG Viewer - " + fi.fileName());

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

void MainWindow::clearColorsEvent() {
  
}
