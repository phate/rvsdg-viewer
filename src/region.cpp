#include <stdio.h>
#include <QDebug>

#include "element.h"
#include "region.h"
#include "argument.h"
#include "result.h"

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

Region::~Region() {
  for(auto it : arguments) {
    delete it;
  }
  for(auto layer : layers) {
    delete layer;
  }
}

void Region::appendItems(QGraphicsItem *parent) {

  //-----------------------------------------------------------------------------
  // build layers for this region

  layer();

  int rows = layers.size();

  height = 0;
  width = 0;

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
      unsigned w = vertex->getWidth();
      if(w > columnWidths[i]) columnWidths[i] = w;
      i++;
    }
  }

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

  unsigned xx = columnSpacing[0];
  unsigned yy = 2*LINE_CLEARANCE;

  unsigned row = rows-1;

  for(auto layer = layers.rbegin(); layer != layers.rend(); layer++) {

    unsigned maxHeight = 0;

    unsigned col = 0;
    currentRoutingYs[row] = yy - LINE_CLEARANCE;

    for(auto vertex : *(*layer)) {
      unsigned w = vertex->getWidth();
      unsigned h = vertex->getHeight();

      unsigned xpos = (xx+(columnWidths[col]-w)/2);
      if(xpos % INPUTOUTPUT_CLEARANCE) {
        xpos += INPUTOUTPUT_CLEARANCE - (xpos % INPUTOUTPUT_CLEARANCE);
      }

      vertex->setPos(xpos, yy);

      xx += columnSpacing[col+1] + columnWidths[col];
      if((xpos + w) > width) width = xpos + w;
      if(h > maxHeight) maxHeight = h;
      col++;
    }

    xx = columnSpacing[0];
    yy += rowSpacing[row] + maxHeight;

    row--;
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

  width += LINE_CLEARANCE;
  height = yy + LINE_CLEARANCE;
}

