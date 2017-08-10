/******************************************************************************
 *
 * QGraphicsScene subclass, used for displaying the RVSDG graph
 *
 * Asbjørn Djupdal 2017
 *
 *****************************************************************************/

#ifndef DIAGRAMSCENE_H
#define DIAGRAMSCENE_H

#include <QGraphicsScene>
#include "rvsdg.h"

#define DEFINED_COLORS 5

class DiagramScene : public QGraphicsScene {
  Q_OBJECT

  unsigned colorCounter;
  QColor colors[DEFINED_COLORS];
  unsigned zvalue;
  Element *lastElement;

public:
  explicit DiagramScene(QObject *parent = 0);
  ~DiagramScene() {}
  void drawElement(Element *element);
  void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent);
};

#endif
