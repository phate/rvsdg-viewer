#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QTreeView>
#include <QMainWindow>
#include "diagramscene.h"
#include "rvsdg.h"

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow();
  MainWindow(const QString &fileName);

protected:
  void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private slots:
  void open();
  void about();
  void regionClicked(const QModelIndex &index);

private:
  void init();
  void loadFile(const QString &fileName);

  QTreeView *treeView;
  DiagramScene *scene;
  QGraphicsView *graphicsView;
  Model *rvsdgModel;

  QMenu *fileMenu;
  QMenu *helpMenu;
  QToolBar *fileToolBar;
  QAction *openAct;
  QAction *exitAct;
  QAction *aboutAct;
  QAction *aboutQtAct;
};

#endif
