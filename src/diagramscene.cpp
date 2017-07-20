#include "diagramscene.h"

#include <QTextCursor>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsEllipseItem>

DiagramScene::DiagramScene(QObject *parent) : QGraphicsScene(parent) {
  QGraphicsItem *item = new QGraphicsEllipseItem(QRectF(QPointF(10,10),QSizeF(100,100)));
  addItem(item);
}
