/******************************************************************************
 *
 * QGraphicsView subclass
 *
 * Asbjørn Djupdal 2017
 *
 *****************************************************************************/

#ifndef DIAGRAMVIEW_H
#define DIAGRAMVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QMouseEvent>

#define SCALE_IN_FACTOR 1.25
#define SCALE_OUT_FACTOR 0.8

class DiagramView : public QGraphicsView {

  Q_OBJECT

public:
  DiagramView(QGraphicsScene *scene) : QGraphicsView(scene) {
  }

public slots:
  void zoomInEvent() {
    scale(SCALE_IN_FACTOR, SCALE_IN_FACTOR);
  }
  void zoomOutEvent() {
    scale(SCALE_OUT_FACTOR, SCALE_OUT_FACTOR);
  }
};

#endif
