/******************************************************************************
 *
 * RVSDG region argument
 *
 * Asbjørn Djupdal 2017
 *
 *****************************************************************************/

#ifndef ARGUMENT_H
#define ARGUMENT_H

#include "element.h"

class Argument : public Element {
  QGraphicsPolygonItem *baseItem;

public:
  Argument(QString id, Element *parent) : Element(id, 0, parent) {}
  unsigned getWidth() {
    return INPUTOUTPUT_SIZE;
  }
  unsigned getHeight() {
    return INPUTOUTPUT_SIZE;
  }
  void appendItems(QGraphicsItem *parent) {
    lineSegments.clear();

    QPolygonF polygon;
    polygon << QPointF(0,0)
            << QPointF(-INPUTOUTPUT_SIZE/2,-INPUTOUTPUT_SIZE)
            << QPointF(INPUTOUTPUT_SIZE/2,-INPUTOUTPUT_SIZE);

    baseItem = new QGraphicsPolygonItem(polygon, parent);
    baseItem->setData(0, QVariant::fromValue((void*)this));
  }
  void setPos(unsigned x, unsigned y) {
    Element::setPos(x + INPUTOUTPUT_SIZE/2, y + INPUTOUTPUT_SIZE);
    baseItem->setPos(x + INPUTOUTPUT_SIZE/2, y + INPUTOUTPUT_SIZE);
  }
};

#endif
