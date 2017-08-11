/******************************************************************************
 *
 * RVSDG node input
 *
 * Asbjørn Djupdal 2017
 *
 *****************************************************************************/

#ifndef INPUT_H
#define INPUT_H

#include "element.h"

class Input : public Element {
  QGraphicsPolygonItem *baseItem;

public:
  Input(QString id, Element *parent) : Element(id, 0, parent) {}
  Element *getVertex() {
    return parent;
  }
  unsigned getRow() {
    return parent->getRow();
  }
  unsigned getColumn() {
    return parent->getColumn();
  }
  unsigned getX() {
    return parent->getX() + x;
  }
  unsigned getY() {
    return parent->getY() + y;
  }
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
    Element::setPos(x, y);
    baseItem->setPos(x, y);
  }
};

#endif
