#include <stdio.h>
#include <QDebug>

#include "rvsdg.h"
#include <iostream>

#include "diagramscene.h"

///////////////////////////////////////////////////////////////////////////////

int Element::constructFromXml(const QDomElement &element, int treeviewRow, std::map<QString,Element*> &elements) {
  Element *child = this;

  QString childId = element.attribute(ATTR_ID, "");
  QString childName = element.attribute(ATTR_NAME, "");
  QString childTypeS = element.attribute(ATTR_TYPE, "");

  NodeType childType = NODE;
  if(childTypeS == "lambda") childType = LAMBDA;
  else if(childTypeS == "gamma") childType = GAMMA;
  else if(childTypeS == "theta") childType = THETA;
  else if(childTypeS == "phi") childType = PHI;

  // FIXME: dangerous casting, will segfault with invalid XML
  if(element.tagName() == TAG_NODE) {
    child = new Node(childId, childName, childType, treeviewRow++, this);
    appendChild(child);

  } else if(element.tagName() == TAG_REGION) {
    child = new Region(childId, treeviewRow++, this);
    appendChild(child);

  } else if(element.tagName() == TAG_INPUT) {
    child = new Input(childId, this);
    ((Node*)this)->appendInput(child);

  } else if(element.tagName() == TAG_OUTPUT) {
    child = new Output(childId, this);
    ((Node*)this)->appendOutput(child);

  } else if(element.tagName() == TAG_ARGUMENT) {
    child = new Argument(childId, this);
    ((Region*)this)->appendArgument(child);

  } else if(element.tagName() == TAG_RESULT) {
    child = new Result(childId, this);
    ((Region*)this)->appendResult(child);
  }

  if(child != this) elements[childId] = child;

  QDomNode n = element.firstChild();
  int i = 0;
  while(!n.isNull()) {
    QDomElement e = n.toElement();
    if(!e.isNull()) i = child->constructFromXml(e, i, elements);
    n = n.nextSibling();
  }

  return treeviewRow;
}

///////////////////////////////////////////////////////////////////////////////

void Node::appendItems(QGraphicsItem *parent) {
  height = NODE_HEIGHT;

  width = inputs.size() * (INPUTOUTPUT_CLEARANCE + INPUTOUTPUT_SIZE);
  unsigned outputWidth = outputs.size() * (INPUTOUTPUT_CLEARANCE + INPUTOUTPUT_SIZE);
  if(outputWidth > width) width = outputWidth;

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

  QGraphicsTextItem *text = new QGraphicsTextItem(name, baseItem);
  text->setPos(QPointF(TEXT_CLEARANCE, 50-text->boundingRect().height()));
  text->setData(0, QVariant::fromValue((void*)this));
  unsigned textwidth = text->boundingRect().width() + TEXT_CLEARANCE*2;
  if(textwidth > width) width = textwidth;

  text = new QGraphicsTextItem(id, baseItem);
  text->setPos(QPointF(TEXT_CLEARANCE, 50));
  text->setData(0, QVariant::fromValue((void*)this));
  textwidth = text->boundingRect().width() + TEXT_CLEARANCE*2;
  if(textwidth > width) width = textwidth;

  //------------------------------------------------------------------------------

  unsigned regionX = 0;

  if(expanded) {
    regionX = REGION_CLEARANCE;

    for(auto child : children) {
      QGraphicsPolygonItem *poly = new QGraphicsPolygonItem(baseItem);
      poly->setBrush(QBrush(QColor(Qt::white)));
    
      child->appendItems(poly);
      poly->setPos(regionX, 100);

      regionX += child->getWidth() + REGION_CLEARANCE;
      if(height < (100 + REGION_CLEARANCE + child->getHeight())) height = 100 + REGION_CLEARANCE + child->getHeight();

      QPolygonF polygon;
      polygon << QPointF(0, 0)
              << QPointF(child->getWidth(), 0)
              << QPointF(child->getWidth(), child->getHeight())
              << QPointF(0, child->getHeight());

      poly->setPolygon(polygon);
    }
  }

  if(regionX > width) width = regionX;

  //------------------------------------------------------------------------------

  int xx = INPUTOUTPUT_CLEARANCE;

  for(auto input : inputs) {
    Q_UNUSED(input);
    QPolygonF p;
    p << QPointF(0,0)
      << QPointF(-INPUTOUTPUT_SIZE/2,INPUTOUTPUT_SIZE)
      << QPointF(INPUTOUTPUT_SIZE/2,INPUTOUTPUT_SIZE);
    QGraphicsPolygonItem *inputItem = new QGraphicsPolygonItem(p, baseItem);
    inputItem->setData(0, QVariant::fromValue((void*)input));
    inputItem->setPos(QPointF(xx, 0));
    xx += INPUTOUTPUT_CLEARANCE + INPUTOUTPUT_SIZE;
    input->setPos(xx-2*INPUTOUTPUT_CLEARANCE, 0);
  }

  xx = INPUTOUTPUT_CLEARANCE + INPUTOUTPUT_CLEARANCE/2;

  for(auto output : outputs) {
    Q_UNUSED(output);
    QPolygonF p;
    p << QPointF(0,0)
      << QPointF(-INPUTOUTPUT_SIZE/2,-INPUTOUTPUT_SIZE)
      << QPointF(INPUTOUTPUT_SIZE/2,-INPUTOUTPUT_SIZE);
    QGraphicsPolygonItem *outputItem = new QGraphicsPolygonItem(p, baseItem);
    outputItem->setData(0, QVariant::fromValue((void*)output));
    outputItem->setPos(QPointF(xx, height + INPUTOUTPUT_CLEARANCE));
    xx += INPUTOUTPUT_CLEARANCE + INPUTOUTPUT_SIZE;
    output->setPos(xx-2*INPUTOUTPUT_CLEARANCE, height + INPUTOUTPUT_CLEARANCE);
  }
  if(outputs.size()) height += INPUTOUTPUT_SIZE;

  //------------------------------------------------------------------------------

  QPolygonF polygon;
  polygon << QPointF(0, 0)
          << QPointF(width, 0)
          << QPointF(width, height)
          << QPointF(0, height);
  baseItem->setPolygon(polygon);
}

///////////////////////////////////////////////////////////////////////////////

/******************************************************************************
 * longest path layering algorithm
 * builds the layers bottom-up (layer 0 is bottom layer)
 *****************************************************************************/
void Region::layer() {
  if(!layers.size()) {
    std::vector<Element*> unassignedNodes = children;
    std::vector<Element*> assignedNodes;
    std::vector<Element*> nodesBelowCurrent;

    unsigned currentLayer = 0;

    // --------------------------------------------------------------------------
    // layer 0 set to all results

    layers.push_back(new std::vector<Element*>);

    for(auto node = results.begin(); node != results.end(); node++) {
      (*node)->setRowCol(currentLayer, layers[currentLayer]->size());
      layers[currentLayer]->push_back(*node);
      nodesBelowCurrent.push_back(*node);
    }

    // new layer, we don't want more nodes in this layer
    currentLayer++;

    // --------------------------------------------------------------------------
    // layer 1 filled with nodes without outgoing edges

    for(auto node = unassignedNodes.begin(); node != unassignedNodes.end();) {
      if(!(*node)->getNumEdges()) {
        if(layers.size() != (currentLayer+1)) {
          layers.push_back(new std::vector<Element*>);
        }

        (*node)->setRowCol(currentLayer, layers[currentLayer]->size());

        layers[currentLayer]->push_back(*node);
        assignedNodes.push_back(*node);
        node = unassignedNodes.erase(node);
      } else {
        node++;
      }
    }

    // --------------------------------------------------------------------------
    // layers 1-n set to the rest of the nodes

    while(unassignedNodes.size()) {
      bool foundNode = false;
      // find node where all successors are in nodesBelowCurrent
      for(auto node = unassignedNodes.begin(); node != unassignedNodes.end();) {
        bool nodeIsEligible = true;
        // look through all outgoing edges of this node
        for(unsigned i = 0; i < (*node)->getNumEdges(); i++) {
          auto successor = (*node)->getEdge(i)->getVertex();
          if(std::find(nodesBelowCurrent.begin(), nodesBelowCurrent.end(), successor) == nodesBelowCurrent.end()) {
            // outgoing edge is not going to a layer below this one, can't choose this node
            nodeIsEligible = false;
            break;
          }
        }
        if(nodeIsEligible) {
          // found eligible node
          foundNode = true;

          if(layers.size() != (currentLayer+1)) {
            layers.push_back(new std::vector<Element*>);
          }

          (*node)->setRowCol(currentLayer, layers[currentLayer]->size());

          layers[currentLayer]->push_back(*node);
          assignedNodes.push_back(*node);
          node = unassignedNodes.erase(node);
        } else {
          node++;
        }
      }
      if(!foundNode) {
        // assert(layers[currentLayer]->size() != 0)
        currentLayer++;
        nodesBelowCurrent.insert(nodesBelowCurrent.end(), assignedNodes.begin(), assignedNodes.end());
        assignedNodes.clear();
      }
    }

    currentLayer = layers.size()-1;

    // --------------------------------------------------------------------------
    // layer n+1 set to all arguments

    if(arguments.size()) {
      currentLayer++;
      layers.push_back(new std::vector<Element*>);
      for(auto node = arguments.begin(); node != arguments.end(); node++) {
        (*node)->setRowCol(currentLayer, layers[currentLayer]->size());
        layers[currentLayer]->push_back(*node);
      }
    }
  }
}

void Region::appendItems(QGraphicsItem *parent) {
  layer();

  // build all vertex graphics items
  for(auto layer = layers.rbegin(); layer != layers.rend(); layer++) {
    for(auto vertex : *(*layer)) {
      vertex->appendItems(parent);
    }
  }

  // find column widths
  std::vector<unsigned> columnWidths(0);
  for(auto layer = layers.rbegin(); layer != layers.rend(); layer++) {
    // this layer has more columns, increase columnWidths vector
    if((*layer)->size() > columnWidths.size()) {
      columnWidths.resize((*layer)->size(), 0);
    }
    // go through all vertices in this layer and update columnWidths if necessary
    int i = 0;
    for(auto vertex : *(*layer)) {
      unsigned width = vertex->getWidth();
      if(width > columnWidths[i]) columnWidths[i] = width;
      i++;
    }
  }

  int rows = layers.size();
  int columns = columnWidths.size();

  std::vector<unsigned> rowSpacing(rows, LINE_CLEARANCE);
  std::vector<unsigned> columnSpacing(columns+1, LINE_CLEARANCE);

  // find row and column spacing
  for(auto layer : layers) {
    for(auto vertex : *layer) {
      for(unsigned i = 0; i < vertex->getNumEdges(); i++) {
        unsigned sourceRow = vertex->getRow();
        unsigned targetRow = vertex->getEdge(i)->getRow();
        unsigned targetColumn = vertex->getEdge(i)->getColumn();

        rowSpacing[sourceRow] += LINE_CLEARANCE;

        if((sourceRow - targetRow) > 1) {
          rowSpacing[targetRow+1] += LINE_CLEARANCE;
          columnSpacing[targetColumn] += LINE_CLEARANCE;
        }
      }
    }
  }
  columnSpacing[columns] = 0;

  // vertical edge routing corridors
  std::vector<unsigned> currentRoutingXs;
  currentRoutingXs.clear();
  currentRoutingXs.push_back(columnSpacing[0] - LINE_CLEARANCE);
  for(unsigned i = 1; i < columnWidths.size(); i++) {
    currentRoutingXs.push_back(currentRoutingXs[i-1] + columnSpacing[i] + columnWidths[i-1]);
  }

  // horizontal edge routing corridors
  std::vector<unsigned> currentRoutingYs;
  currentRoutingYs.clear();
  currentRoutingYs.resize(layers.size(), 0);

  // display vertices
  unsigned x = columnSpacing[0];
  unsigned y = 2*LINE_CLEARANCE;
  unsigned maxHeight = 0;

  this->width = 0;

  int l = layers.size() - 1;

  int row = rows-1;
  for(auto layer = layers.rbegin(); layer != layers.rend(); layer++) {
    int col = 0;
    currentRoutingYs[l--] = y - LINE_CLEARANCE;
    for(auto vertex : *(*layer)) {

      // set vertex position
      int width = vertex->getWidth();
      int posX = x+(columnWidths[col]-width)/2;
      vertex->setPos(posX, y);

      // add graphics item for this vertex
      unsigned height = vertex->getHeight();

      x += columnSpacing[col+1] + columnWidths[col];
      if(x > this->width) this->width = x;
      if(height > maxHeight) maxHeight = height;
      col++;
    }

    x = columnSpacing[0];
    y += rowSpacing[row--] + maxHeight;
  }

  // display edges
  for(auto layer : layers) {
    for(auto vertex : *layer) {
      vertex->clearLineSegments();

      for(unsigned i = 0; i < vertex->getNumEdges(); i++) {
        Element *source = NULL;
        Element *target = vertex->getEdge(i, &source);

        std::vector<QGraphicsLineItem*> lines(0);

        if((source->getRow() - target->getRow()) > 1) {
          // edge is spanning more than one row, add polyline
          unsigned currentRoutingX = currentRoutingXs[target->getColumn()];
          unsigned currentRoutingYSource = currentRoutingYs[source->getRow()-1];
          unsigned currentRoutingYTarget = currentRoutingYs[target->getRow()];

          lines.resize(5);

          lines.push_back(new QGraphicsLineItem(source->getX(), source->getY(), source->getX(), currentRoutingYSource, parent));
          lines.push_back(new QGraphicsLineItem(source->getX(), currentRoutingYSource, currentRoutingX, currentRoutingYSource, parent));
          lines.push_back(new QGraphicsLineItem(currentRoutingX, currentRoutingYSource, currentRoutingX, currentRoutingYTarget, parent));
          lines.push_back(new QGraphicsLineItem(currentRoutingX, currentRoutingYTarget, target->getX(), currentRoutingYTarget, parent));
          lines.push_back(new QGraphicsLineItem(target->getX(), currentRoutingYTarget, target->getX(), target->getY(), parent));
      
          currentRoutingXs[target->getColumn()] -= LINE_CLEARANCE;
          currentRoutingYs[source->getRow()-1] -= LINE_CLEARANCE;
          currentRoutingYs[target->getRow()] -= LINE_CLEARANCE;

        } else {
          unsigned currentRoutingY = currentRoutingYs[source->getRow()-1];

          lines.resize(3);

          lines.push_back(new QGraphicsLineItem(source->getX(), source->getY(), source->getX(), currentRoutingY, parent));
          lines.push_back(new QGraphicsLineItem(source->getX(), currentRoutingY, target->getX(), currentRoutingY, parent));
          lines.push_back(new QGraphicsLineItem(target->getX(), currentRoutingY, target->getX(), target->getY(), parent));
      
          currentRoutingYs[source->getRow()-1] -= LINE_CLEARANCE;
        }

        target->clearLineSegments();
        target->setLineSegments(i, lines);
        vertex->setLineSegments(i, lines);
      }
    }
  }

  this->height = y - maxHeight + LINE_CLEARANCE;
}

///////////////////////////////////////////////////////////////////////////////

void Argument::appendItems(QGraphicsItem *parent) {
  QPolygonF polygon;
  polygon << QPointF(0,0)
          << QPointF(-INPUTOUTPUT_SIZE/2,-INPUTOUTPUT_SIZE)
          << QPointF(INPUTOUTPUT_SIZE/2,-INPUTOUTPUT_SIZE);

  baseItem = new QGraphicsPolygonItem(polygon, parent);
  baseItem->setData(0, QVariant::fromValue((void*)this));
}

///////////////////////////////////////////////////////////////////////////////

void Result::appendItems(QGraphicsItem *parent) {
  QPolygonF polygon;
  polygon << QPointF(0,0)
          << QPointF(-INPUTOUTPUT_SIZE/2,INPUTOUTPUT_SIZE)
          << QPointF(INPUTOUTPUT_SIZE/2,INPUTOUTPUT_SIZE);

  baseItem = new QGraphicsPolygonItem(polygon, parent);
  baseItem->setData(0, QVariant::fromValue((void*)this));
}

