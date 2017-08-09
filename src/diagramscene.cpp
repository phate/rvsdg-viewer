#include "diagramscene.h"
#include "rvsdg.h"

#include <QTextCursor>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsEllipseItem>

DiagramScene::DiagramScene(QObject *parent) : QGraphicsScene(parent) {
  zvalue = 0;
  colorCounter = 0;

  colors[0] = Qt::red;
  colors[1] = Qt::green;
  colors[2] = Qt::blue;
  colors[3] = Qt::yellow;
  colors[4] = Qt::cyan;
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
  if (mouseEvent->button() == Qt::LeftButton) {
    QGraphicsItem *item = itemAt(mouseEvent->scenePos(), QTransform());
    Element *el = (Element*)item->data(0).value<void*>();
    if(el) {
      std::vector<QGraphicsLineItem*> lines = el->getLineSegments();
      for(auto line : lines) {
        if(line) {
          line->setPen(QPen(colors[colorCounter]));
          line->setZValue(zvalue++);
        }
      }
      colorCounter = (colorCounter + 1) % DEFINED_COLORS;
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
