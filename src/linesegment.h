#ifndef LINESEGMENT_H
#define LINESEGMENT_H

#include <QGraphicsLineItem>
#include "edge.h"

class LineSegment {
public:
  Edge *edge;
  QGraphicsLineItem *item;

  LineSegment(Edge *edge, QGraphicsLineItem *item) {
    this->edge = edge;
    this->item = item;
  }
};

#endif
