/******************************************************************************
 *
 * Main window
 *
 * Asbjørn Djupdal 2017
 *
 *****************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>
#include <QTreeView>
#include <QMainWindow>

#include "diagramscene.h"
#include "element.h"
#include "model.h"

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow();
  MainWindow(const QString &fileName);
  ~MainWindow();

protected:
  void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private slots:
  void open();
  void about();
  void clearColorsEvent();
  void regionClicked(const QModelIndex &index);

private:
  void init();
  void loadFile(const QString &fileName);

  QTreeView *treeView;
  DiagramScene *scene;
  QGraphicsView *graphicsView;
  QComboBox *colorBox;
  Model *rvsdgModel;
  QSplitter *splitter;
  QMenu *fileMenu;
  QMenu *helpMenu;
  QToolBar *fileToolBar;
  QAction *openAct;
  QAction *exitAct;
  QAction *aboutAct;
  QAction *aboutQtAct;
  QAction *zoomInAct;
  QAction *zoomOutAct;
  QAction *clearColorsAct;
};

#endif
