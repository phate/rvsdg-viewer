#include "diagramscene.h"

#include <QTextCursor>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsEllipseItem>

QColor edgeColors[] = EDGE_COLORS;

DiagramScene::DiagramScene(QObject *parent) : QGraphicsScene(parent) {
  zvalue = 0;
  colorCounter = 0;
}

void DiagramScene::drawElement(Element *element) {
  lastElement = element;

  zvalue = 0;
  colorCounter = 0;

  clear();

  QGraphicsLineItem *item = new QGraphicsLineItem();
  item->setPos(0,0);
  element->appendItems(item);
  element->setPos(0,0);

  addItem(item);

  setSceneRect(QRectF(0, 0, element->getWidth(), element->getHeight()));
}

void DiagramScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) {
  if((mouseEvent->button() == Qt::LeftButton) || (mouseEvent->button() == Qt::RightButton)) {
    QGraphicsItem *item = itemAt(mouseEvent->scenePos(), QTransform());
    Element *el = (Element*)item->data(0).value<void*>();
    if(el) {
      std::vector<QGraphicsLineItem*> lines = el->getLineSegments();
      for(auto line : lines) {
        if(line) {
          if(mouseEvent->button() == Qt::LeftButton) {
            line->setPen(QPen(edgeColors[colorCounter]));
          } else {
            line->setPen(QPen(Qt::black));
          }
          line->setZValue(zvalue++);
        }
      }
      colorCounter = (colorCounter + 1) % (sizeof(edgeColors)/sizeof(edgeColors[0]));
    }
  }
}

void DiagramScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent) {
  if (mouseEvent->button() == Qt::LeftButton) {
    QGraphicsItem *item = itemAt(mouseEvent->scenePos(), QTransform());
    Element *el = (Element*)item->data(0).value<void*>();
    if(el) {
      if(el->isComplexNode()) {
        Node *node = (Node*)el;
        node->toggleExpanded();
        drawElement(lastElement);
      }
    }
  }
}
