#ifndef DIAGRAMSCENE_H
#define DIAGRAMSCENE_H

#include <QGraphicsScene>
#include "rvsdg.h"

class DiagramScene : public QGraphicsScene {
  Q_OBJECT

  unsigned colorCounter;
  QColor colors[10];
  unsigned zvalue;
  Region *lastRegion;

public:
  explicit DiagramScene(QObject *parent = 0);
  ~DiagramScene();
  void drawRegion(Region *region);
  void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent);
};

#endif
