/******************************************************************************
 *
 * QGraphicsScene subclass, used for displaying the RVSDG graph
 *
 * Asbjørn Djupdal 2017
 *
 *****************************************************************************/

#ifndef DIAGRAMSCENE_H
#define DIAGRAMSCENE_H

#include <QtWidgets>
#include <QGraphicsScene>
#include "node.h"

class DiagramScene : public QGraphicsScene {
  Q_OBJECT

  unsigned zvalue;
  Element *lastElement;
  QComboBox *colorBox;

public:
  explicit DiagramScene(QComboBox *colorBox, QObject *parent = 0);
  ~DiagramScene() {}
  void drawElement(Element *element);
  void redraw() {
    if(lastElement) {
      drawElement(lastElement);
    }
  }
  void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent);
};

#endif
