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
#include <QApplication>

#include "rvsdg-viewer.h"

class DiagramView : public QGraphicsView {

  Q_OBJECT

protected:
  void mouseReleaseEvent(QMouseEvent *event) {
    QGraphicsView::mouseReleaseEvent(event);
    viewport()->setCursor(Qt::ArrowCursor);
  }
  void wheelEvent(QWheelEvent *event) {
    if(event->modifiers() & Qt::ControlModifier) {
      if(event->delta() > 0) zoomInEvent();
      else zoomOutEvent();
      event->accept();
    } else {
      QGraphicsView::wheelEvent(event);
    }
  }
  void keyPressEvent(QKeyEvent *event) {
    if(event->modifiers() & Qt::ControlModifier) {
      if(event->key() == Qt::Key_Plus) {
        zoomInEvent();
      }
      if(event->key() == Qt::Key_Minus) {
        zoomOutEvent();
      }
    }
    event->accept();
  }

public:
  DiagramView(QGraphicsScene *scene) : QGraphicsView(scene) {
    setDragMode(QGraphicsView::ScrollHandDrag);
    viewport()->setCursor(Qt::ArrowCursor);
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
