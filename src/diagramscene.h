#ifndef DIAGRAMSCENE_H
#define DIAGRAMSCENE_H

#include <QGraphicsScene>
#include "rvsdg.h"

class DiagramScene : public QGraphicsScene {
  Q_OBJECT

public:
  explicit DiagramScene(QObject *parent = 0);
  ~DiagramScene();
  void drawRegion(Region *region);
};

class GfxNode : public QGraphicsPolygonItem {
public:
  GfxNode(QString id, unsigned width) {
    QGraphicsTextItem *text = new QGraphicsTextItem(id, this);
    text->setPos(QPointF(TEXT_CLEARANCE, NODE_HEIGHT/2-text->boundingRect().height()/2));

    QPolygonF polygon;
    polygon << QPointF(0,0)
            << QPointF(width,0)
            << QPointF(width,NODE_HEIGHT)
            << QPointF(0,NODE_HEIGHT);
    setPolygon(polygon);
  }
};

class GfxInput : public QGraphicsPolygonItem {
public:
  GfxInput(QGraphicsPolygonItem *parent) : QGraphicsPolygonItem(parent) {
    QPolygonF polygon;
    polygon << QPointF(0,0)
            << QPointF(-INPUTOUTPUT_SIZE/2,INPUTOUTPUT_SIZE)
            << QPointF(INPUTOUTPUT_SIZE/2,INPUTOUTPUT_SIZE);
    setPolygon(polygon);
  }
};

class GfxOutput : public QGraphicsPolygonItem {
public:
  GfxOutput(QGraphicsPolygonItem *parent) : QGraphicsPolygonItem(parent) {
    QPolygonF polygon;
    polygon << QPointF(0,0)
            << QPointF(-INPUTOUTPUT_SIZE/2,-INPUTOUTPUT_SIZE)
            << QPointF(INPUTOUTPUT_SIZE/2,-INPUTOUTPUT_SIZE);
    setPolygon(polygon);
  }
};

class GfxArgument : public QGraphicsPolygonItem {
public:
  GfxArgument() {
    QPolygonF polygon;
    polygon << QPointF(0,0)
            << QPointF(-INPUTOUTPUT_SIZE/2,INPUTOUTPUT_SIZE)
            << QPointF(INPUTOUTPUT_SIZE/2,INPUTOUTPUT_SIZE);
    setPolygon(polygon);
  }
};

class GfxResult : public QGraphicsPolygonItem {
public:
  GfxResult() {
    QPolygonF polygon;
    polygon << QPointF(0,0)
            << QPointF(-INPUTOUTPUT_SIZE/2,-INPUTOUTPUT_SIZE)
            << QPointF(INPUTOUTPUT_SIZE/2,-INPUTOUTPUT_SIZE);
    setPolygon(polygon);
  }
};

#endif
