#include <stdio.h>
#include <QDebug>
#include <QBrush>

#include "node.h"
#include "input.h"
#include "output.h"

Element *Node::parseXmlElement(QString tagName, QString childId) {
  Element *child = this;

  if(tagName == TAG_INPUT) {
    child = new Input(childId, this);
    appendInput(child);

  } else if(tagName == TAG_OUTPUT) {
    child = new Output(childId, this);
    appendOutput(child);
  }

  return child;
}

Node::~Node() {
  for(auto it : inputs) {
    delete it;
  }
  for(auto it : outputs) {
    delete it;
  }
}

unsigned Node::getNumEdges() {
  int totalSize = 0;
  for(auto it : outputs) {
    totalSize += it->getNumEdges();
  }
  return totalSize;
}

Element *Node::getEdge(unsigned n, Element **source) {
  for(auto it : outputs) {
    if(n < it->getNumEdges()) {
      if(source) *source = it;
      return it->getEdge(n);
    }
    n -= it->getNumEdges();
  }
  return NULL;
}

void Node::setLineSegments(unsigned n, std::vector<QGraphicsLineItem*>lines) {
  for(auto it : outputs) {
    if(n < it->getNumEdges()) {
      it->setLineSegments(n, lines);
    }
    n -= it->getNumEdges();
  }
}

void Node::clearLineSegments() {
  for(auto it : outputs) {
    it->clearLineSegments();
  }
  for(auto it : inputs) {
    it->clearLineSegments();
  }
}

QString Node::getTypeName() {
  switch(type) {
    case LAMBDA: 
      return QString("Lambda");
    case GAMMA: 
      return QString("Gamma");
    case THETA: 
      return QString("Theta");
    case PHI: 
      return QString("Phi");
    default:
      return QString("Node");
  }
  return QString("Node");
}

void Node::appendItems(QGraphicsItem *parent) {
  unsigned xx = 0;
  unsigned yy = 0;

  width = 0;
  height = 0;

  // create base item (rectangle)
  baseItem = new QGraphicsPolygonItem(parent);
  baseItem->setPos(QPointF(x, y));
  baseItem->setData(0, QVariant::fromValue((void*)this));

  switch(type) {
    case LAMBDA:
      baseItem->setBrush(QBrush(QColor(LAMBDA_NODE_COLOR)));
      break;
    case GAMMA:
      baseItem->setBrush(QBrush(QColor(GAMMA_NODE_COLOR)));
      break;
    case THETA:
      baseItem->setBrush(QBrush(QColor(THETA_NODE_COLOR)));
      break;
    case PHI:
      baseItem->setBrush(QBrush(QColor(PHI_NODE_COLOR)));
      break;
    default:
      baseItem->setBrush(QBrush(QColor(NODE_COLOR)));
      break;
  }

  // create input items
  xx = INPUTOUTPUT_CLEARANCE;
  for(auto input : inputs) {
    input->appendItems(baseItem);
    input->setPos(xx + (input->getWidth()/2), yy);
    xx += INPUTOUTPUT_CLEARANCE + input->getWidth();;
  }
  if(inputs.size()) {
    yy += inputs[0]->getHeight();
  }
  if(xx > width) width = xx;

  // create name text
  xx = TEXT_CLEARANCE;
  yy += TEXT_CLEARANCE;
  QGraphicsTextItem *text = new QGraphicsTextItem(name, baseItem);
  text->setPos(QPointF(xx, yy));
  text->setData(0, QVariant::fromValue((void*)this));
  unsigned textwidth = text->boundingRect().width() + TEXT_CLEARANCE*2;
  if(textwidth > width) width = textwidth;
  yy += text->boundingRect().height();

  // create id text
  xx = TEXT_CLEARANCE;
  yy += TEXT_CLEARANCE;
  text = new QGraphicsTextItem(id, baseItem);
  text->setPos(QPointF(xx, yy));
  text->setData(0, QVariant::fromValue((void*)this));
  textwidth = text->boundingRect().width() + TEXT_CLEARANCE*2;
  if(textwidth > width) width = textwidth;
  yy += text->boundingRect().height();

  //------------------------------------------------------------------------------
  // for expanded nodes, create items for all children (layed out horizontally)

  if(expanded) {
    unsigned maxHeight = 0;

    xx = REGION_CLEARANCE;
    yy += REGION_CLEARANCE;

    for(auto child : children) {
      // create a rectangle for the child region
      QGraphicsPolygonItem *poly = new QGraphicsPolygonItem(baseItem);
      poly->setBrush(QBrush(QColor(Qt::white)));
    
      // create child region
      child->appendItems(poly);
      poly->setPos(xx, yy);

      // update position variables
      xx += child->getWidth() + REGION_CLEARANCE;
      if(child->getHeight() > maxHeight) maxHeight = child->getHeight();

      // now that region size is known, set the polygon for the region rectangle
      QPolygonF polygon;
      polygon << QPointF(0, 0)
              << QPointF(child->getWidth(), 0)
              << QPointF(child->getWidth(), child->getHeight())
              << QPointF(0, child->getHeight());
      poly->setPolygon(polygon);
    }

    yy += maxHeight;
  }

  if(xx > width) width = xx;

  //------------------------------------------------------------------------------

  // create outupt items
  xx = INPUTOUTPUT_CLEARANCE + INPUTOUTPUT_CLEARANCE/2; // add some to avoid edge overlap
  yy += INPUTOUTPUT_CLEARANCE;
  for(auto output : outputs) {
    output->appendItems(baseItem);
    output->setPos(xx + (output->getWidth()/2), yy + output->getHeight());
    xx += INPUTOUTPUT_CLEARANCE + output->getWidth();
  }
  if(xx > width) width = xx;
  if(outputs.size()) {
    yy += outputs[0]->getHeight();
  }

  //------------------------------------------------------------------------------

  height = yy;

  //------------------------------------------------------------------------------

  // now the total node size is known, create the polygon for the node rectangle
  QPolygonF polygon;
  polygon << QPointF(0, 0)
          << QPointF(width, 0)
          << QPointF(width, height)
          << QPointF(0, height);
  baseItem->setPolygon(polygon);
}
