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

#include "rvsdg-viewer.h"

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
