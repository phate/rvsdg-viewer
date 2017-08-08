#ifndef DIAGRAMSCENE_H
#define DIAGRAMSCENE_H

#include <QGraphicsScene>
#include "rvsdg.h"

class DiagramScene : public QGraphicsScene {
  Q_OBJECT

private:
  unsigned colorCounter;
  QColor colors[10];
  unsigned zvalue;

  std::vector<unsigned> currentRoutingXs;
  std::vector<unsigned> currentRoutingYs;

  void routeEdges(Element *vertex, const QColor &color);

public:
  explicit DiagramScene(QObject *parent = 0);
  ~DiagramScene();
  void drawRegion(Region *region);
  void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
};

#endif
