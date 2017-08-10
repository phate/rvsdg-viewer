#include <stdio.h>
#include <QDebug>

#include "rvsdg.h"
#include <iostream>

#include "diagramscene.h"

///////////////////////////////////////////////////////////////////////////////

/* recursive function that constructs the graph from XML DOM elements */
/* the DOM element represents one child of this object */
int Element::constructFromXml(const QDomElement &element, int treeviewRow, std::map<QString,Element*> &elements) {
  Element *child = this;

  // create child from DOM element
  QString childId = element.attribute(ATTR_ID, "");
  QString tagName = element.tagName();

  if(tagName == TAG_NODE) {
    QString childName = element.attribute(ATTR_NAME, "");
    QString childTypeS = element.attribute(ATTR_TYPE, "");

    NodeType childType = NODE;
    if(childTypeS == "lambda") childType = LAMBDA;
    else if(childTypeS == "gamma") childType = GAMMA;
    else if(childTypeS == "theta") childType = THETA;
    else if(childTypeS == "phi") childType = PHI;

    child = new Node(childId, childName, childType, treeviewRow++, this);
    appendChild(child);

  } else if(tagName == TAG_REGION) {
    child = new Region(childId, treeviewRow++, this);
    appendChild(child);

  } else {
    child = parseXmlElement(tagName, childId);
  }

  // append child to children vector
  if(child != this) elements[childId] = child;

  // loop through all the DOM element children and construct granchildren recursivly
  QDomNode n = element.firstChild();
  int i = 0;
  while(!n.isNull()) {
    QDomElement e = n.toElement();
    if(!e.isNull()) i = child->constructFromXml(e, i, elements);
    n = n.nextSibling();
  }

  return treeviewRow;
}

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

Element *Region::parseXmlElement(QString tagName, QString childId) {
  Element *child = this;

  if(tagName == TAG_ARGUMENT) {
    child = new Argument(childId, this);
    appendArgument(child);

  } else if(tagName == TAG_RESULT) {
    child = new Result(childId, this);
    appendResult(child);
  }

  return child;
}

///////////////////////////////////////////////////////////////////////////////

void Node::appendItems(QGraphicsItem *parent) {
  height = NODE_HEIGHT;

  // calculate width based on number of inputs and outputs
  width = inputs.size() * (INPUTOUTPUT_CLEARANCE + INPUTOUTPUT_SIZE);
  unsigned outputWidth = outputs.size() * (INPUTOUTPUT_CLEARANCE + INPUTOUTPUT_SIZE);
  if(outputWidth > width) width = outputWidth;

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

  // create name text
  QGraphicsTextItem *text = new QGraphicsTextItem(name, baseItem);
  text->setPos(QPointF(TEXT_CLEARANCE, NODE_HEIGHT/2-text->boundingRect().height()));
  text->setData(0, QVariant::fromValue((void*)this));
  unsigned textwidth = text->boundingRect().width() + TEXT_CLEARANCE*2;
  if(textwidth > width) width = textwidth;

  // create id text
  text = new QGraphicsTextItem(id, baseItem);
  text->setPos(QPointF(TEXT_CLEARANCE, NODE_HEIGHT/2));
  text->setData(0, QVariant::fromValue((void*)this));
  textwidth = text->boundingRect().width() + TEXT_CLEARANCE*2;
  if(textwidth > width) width = textwidth;

  //------------------------------------------------------------------------------
  // for expanded nodes, create items for all children (layed out horizontally)

  unsigned regionX = 0;

  if(expanded) {
    regionX = REGION_CLEARANCE;

    for(auto child : children) {
      // create a rectangle for the child region
      QGraphicsPolygonItem *poly = new QGraphicsPolygonItem(baseItem);
      poly->setBrush(QBrush(QColor(Qt::white)));
    
      // create child region
      child->appendItems(poly);
      poly->setPos(regionX, NODE_HEIGHT);

      // update position variables
      regionX += child->getWidth() + REGION_CLEARANCE;
      if(height < (NODE_HEIGHT + REGION_CLEARANCE + child->getHeight())) {
        height = NODE_HEIGHT + REGION_CLEARANCE + child->getHeight();
      }

      // now that region size is known, set the polygon for the region rectangle
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

  // create input items
  int xx = INPUTOUTPUT_CLEARANCE;

  for(auto input : inputs) {
    input->appendItems(baseItem);
    input->setPos(xx, 0);
    xx += INPUTOUTPUT_CLEARANCE + input->getWidth();;
  }

  // create outupt items
  xx = INPUTOUTPUT_CLEARANCE + INPUTOUTPUT_CLEARANCE/2; // add some to avoid edge overlap
  for(auto output : outputs) {
    output->appendItems(baseItem);
    output->setPos(xx, height + INPUTOUTPUT_CLEARANCE);
    xx += INPUTOUTPUT_CLEARANCE + output->getWidth();
  }
  if(outputs.size()) height += outputs[0]->getHeight();

  //------------------------------------------------------------------------------

  // now the total node size is known, create the polygon for the node rectangle
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

  if(!layers.size()) { // don't rebuild unnecessary

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

  //-----------------------------------------------------------------------------
  // build layers for this region

  layer();

  //-----------------------------------------------------------------------------
  // build all vertex graphics items
  // need to do this now so that all vertex sizes are known during placement

  for(auto layer = layers.rbegin(); layer != layers.rend(); layer++) {
    for(auto vertex : *(*layer)) {
      vertex->appendItems(parent);
    }
  }

  //-----------------------------------------------------------------------------
  // calculate mesh positions for vertices and edges

  // find column widths
  std::vector<unsigned> columnWidths(0);
  for(auto layer = layers.rbegin(); layer != layers.rend(); layer++) {
    // this layer has more columns than previous layers, increase columnWidths vector
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
  // this is based on the number of edges that must be routed here
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

  //-----------------------------------------------------------------------------
  // position vertices

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

      unsigned height = vertex->getHeight();

      x += columnSpacing[col+1] + columnWidths[col];
      if(x > this->width) this->width = x;
      if(height > maxHeight) maxHeight = height;
      col++;
    }

    x = columnSpacing[0];
    y += rowSpacing[row--] + maxHeight;
  }

  //-----------------------------------------------------------------------------
  // create and display edges

  for(auto layer : layers) {
    for(auto vertex : *layer) {
      vertex->clearLineSegments();

      for(unsigned i = 0; i < vertex->getNumEdges(); i++) {
        Element *source = NULL;
        Element *target = vertex->getEdge(i, &source);

        std::vector<QGraphicsLineItem*> lines(0);

        if((source->getRow() - target->getRow()) > 1) {
          // edge is spanning more than one row
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
          // edge is between neighbouring rows
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

  this->width += LINE_CLEARANCE;

  this->height = y - maxHeight + LINE_CLEARANCE;
}

///////////////////////////////////////////////////////////////////////////////

void Input::appendItems(QGraphicsItem *parent) {
  QPolygonF polygon;
  polygon << QPointF(0,0)
          << QPointF(-INPUTOUTPUT_SIZE/2,INPUTOUTPUT_SIZE)
          << QPointF(INPUTOUTPUT_SIZE/2,INPUTOUTPUT_SIZE);
  baseItem = new QGraphicsPolygonItem(polygon, parent);
  baseItem->setData(0, QVariant::fromValue((void*)this));
}

///////////////////////////////////////////////////////////////////////////////

void Output::appendItems(QGraphicsItem *parent) {
  QPolygonF polygon;
  polygon << QPointF(0,0)
          << QPointF(-INPUTOUTPUT_SIZE/2,-INPUTOUTPUT_SIZE)
          << QPointF(INPUTOUTPUT_SIZE/2,-INPUTOUTPUT_SIZE);
  baseItem = new QGraphicsPolygonItem(polygon, parent);
  baseItem->setData(0, QVariant::fromValue((void*)this));
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
