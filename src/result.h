/******************************************************************************
 *
 * RVSDG region result
 *
 * Asbjørn Djupdal 2017
 *
 *****************************************************************************/

#ifndef RESULT_H
#define RESULT_H

#include "element.h"

class Result : public Element {
  QGraphicsPolygonItem *baseItem;

public:
  Result(QString id, Element *parent) : Element(id, 0, parent) {}
  unsigned getWidth() {
    return INPUTOUTPUT_SIZE;
  }
  unsigned getHeight() {
    return INPUTOUTPUT_SIZE;
  }
  void appendItems(QGraphicsItem *parent) {
    QPolygonF polygon;
    polygon << QPointF(0,0)
            << QPointF(-INPUTOUTPUT_SIZE/2,INPUTOUTPUT_SIZE)
            << QPointF(INPUTOUTPUT_SIZE/2,INPUTOUTPUT_SIZE);

    baseItem = new QGraphicsPolygonItem(polygon, parent);
    baseItem->setData(0, QVariant::fromValue((void*)this));
  }
  void setPos(unsigned x, unsigned y) {
    Element::setPos(x + INPUTOUTPUT_SIZE, y);
    baseItem->setPos(x + INPUTOUTPUT_SIZE, y);
  }
};

#endif
