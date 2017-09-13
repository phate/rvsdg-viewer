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
#include <QScrollBar>

#include "rvsdg-viewer.h"

class DiagramView : public QGraphicsView {

  Q_OBJECT

protected:
  unsigned originX, originY;

  void wheelEvent(QWheelEvent *event) {
    if(event->modifiers() & Qt::ControlModifier) {
      if(event->delta() > 0) zoomInEvent();
      else zoomOutEvent();
      centerOn(mapToScene(event->pos()));
      event->accept();
    } else {
      QGraphicsView::wheelEvent(event);
    }
  }
  void mousePressEvent(QMouseEvent* event) {
    if(event->button() & Qt::LeftButton) {
      originX = event->x();
      originY = event->y();
      setCursor(Qt::ClosedHandCursor);
    }
    event->accept();
  }
  void mouseReleaseEvent(QMouseEvent* event) {
    if(event->button() & Qt::LeftButton) {
      setCursor(Qt::ArrowCursor);
    }
    event->accept();
  }
  void mouseMoveEvent(QMouseEvent *event) {
    if(event->buttons() & Qt::LeftButton) {
      horizontalScrollBar()->setValue(horizontalScrollBar()->value() - (event->x() - originX));
      verticalScrollBar()->setValue(verticalScrollBar()->value() - (event->y() - originY));
      originX = event->x();
      originY = event->y();
    }
    event->accept();
  }

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
