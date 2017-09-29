#include "diagramscene.h"

#include <QTextCursor>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsEllipseItem>

extern QColor edgeColors[];

DiagramScene::DiagramScene(QComboBox *colorBox, QObject *parent) : QGraphicsScene(parent) {
  this->colorBox = colorBox;
  zvalue = 1;
}

void DiagramScene::drawElement(Element *element) {
  lastElement = element;

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
      std::vector<LineSegment> lines = el->getLineSegments();
      for(auto line : lines) {
        QPen pen = line.item->pen();

        if(mouseEvent->button() == Qt::LeftButton) {
          line.edge->color = colorBox->currentIndex();
          line.edge->zvalue = zvalue++;
          pen.setColor(edgeColors[line.edge->color]);
        } else {
          pen.setColor(Qt::black);
          line.edge->color = -1;
          line.edge->zvalue = 0;
        }

        line.item->setPen(pen);
        line.item->setZValue(line.edge->zvalue);
      }
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
