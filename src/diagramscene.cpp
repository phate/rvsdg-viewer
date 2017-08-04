#include "diagramscene.h"
#include "rvsdg.h"

#include <QTextCursor>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsEllipseItem>

#define SPACING_X 100
#define SPACING_Y 100

#define LINE_CLEARANCE 10

DiagramScene::DiagramScene(QObject *parent) : QGraphicsScene(parent) {
}

DiagramScene::~DiagramScene() {
}

void DiagramScene::drawRegion(Region *region) {

  /////////////////////////////////////////////////////////////////////////////
  // longest path layering algorithm
  // builds the layers bottom-up (layer 0 is bottom layer)

  std::vector<std::vector<Element*>*> layers; // resulting layer matrix

  std::vector<Element*> unassignedNodes = region->children;
  std::vector<Element*> assignedNodes;
  std::vector<Element*> nodesBelowCurrent;

  unsigned currentLayer = 0;

  // --------------------------------------------------------------------------
  // layer 0 set to all results

  layers.push_back(new std::vector<Element*>);

  for(auto node = region->results.begin(); node != region->results.end(); node++) {
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

  if(region->arguments.size()) {
    currentLayer++;
    layers.push_back(new std::vector<Element*>);
    for(auto node = region->arguments.begin(); node != region->arguments.end(); node++) {
      (*node)->setRowCol(currentLayer, layers[currentLayer]->size());
      layers[currentLayer]->push_back(*node);
    }
  }

  ///////////////////////////////////////////////////////////////////////////////
  // display graph
  
  // remove previous items from scene
  clear();

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

  // find vertical edge routing corridors
  std::vector<unsigned> currentRoutingXs;
  currentRoutingXs.push_back(SPACING_X - LINE_CLEARANCE);
  for(unsigned i = 1; i < columnWidths.size(); i++) {
    currentRoutingXs.push_back(currentRoutingXs[i-1] + SPACING_X + columnWidths[i]);
  }

  // find horizontal edge routing corridors
  std::vector<unsigned> currentRoutingYs(layers.size(), 0);

  // display vertices
  unsigned x = SPACING_X;
  unsigned y = SPACING_Y;
  unsigned maxHeight = 0;

  unsigned sceneWidth = 0;

  int l = layers.size() - 1;

  for(auto layer = layers.rbegin(); layer != layers.rend(); layer++) {
    int i = 0;
    currentRoutingYs[l--] = y - LINE_CLEARANCE;
    for(auto vertex : *(*layer)) {

      // set vertex position
      int width = vertex->getWidth();
      int posX = x+(columnWidths[i]-width)/2;
      vertex->setPos(posX, y);

      // add graphics item for this vertex
      unsigned height = vertex->addItem(this);

      x += SPACING_X + columnWidths[i];
      if(x > sceneWidth) sceneWidth = x;
      if(height > maxHeight) maxHeight = height;
      i++;
    }

    x = SPACING_X;
    y += SPACING_Y + maxHeight;
  }

  // display edges
  for(auto layer : layers) {
    for(auto vertex : *layer) {
      for(unsigned i = 0; i < vertex->getNumEdges(); i++) {
        Element *source = NULL;
        Element *target = vertex->getEdge(i, &source);

        if((source->getRow() - target->getRow()) > 1) {
          // edge is spanning more than one row, add polyline
          unsigned currentRoutingX = currentRoutingXs[target->getColumn()];
          unsigned currentRoutingYSource = currentRoutingYs[source->getRow()-1];
          unsigned currentRoutingYTarget = currentRoutingYs[target->getRow()];

          addLine(source->getX(), source->getY(), source->getX(), currentRoutingYSource);
          addLine(source->getX(), currentRoutingYSource, currentRoutingX, currentRoutingYSource);
          addLine(currentRoutingX, currentRoutingYSource, currentRoutingX, currentRoutingYTarget);
          addLine(currentRoutingX, currentRoutingYTarget, target->getX(), currentRoutingYTarget);
          addLine(target->getX(), currentRoutingYTarget, target->getX(), target->getY());

          currentRoutingXs[target->getColumn()] -= LINE_CLEARANCE;
          currentRoutingYs[source->getRow()-1] -= LINE_CLEARANCE;
          currentRoutingYs[target->getRow()] -= LINE_CLEARANCE;

        } else {
          unsigned currentRoutingYSource = currentRoutingYs[source->getRow()-1];

          addLine(source->getX(), source->getY(), source->getX(), currentRoutingYSource);
          addLine(source->getX(), currentRoutingYSource, target->getX(), currentRoutingYSource);
          addLine(target->getX(), currentRoutingYSource, target->getX(), target->getY());

          currentRoutingYs[source->getRow()-1] -= LINE_CLEARANCE;
        }
      }
    }
  }

  setSceneRect(QRectF(0, 0, sceneWidth + SPACING_X, y + SPACING_Y));

  ///////////////////////////////////////////////////////////////////////////////
  // cleanup
  
  for(auto layer : layers) {
    delete layer;
  }
}

void DiagramScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) {
  if (mouseEvent->button() == Qt::LeftButton) {
    QGraphicsItem *item = itemAt(mouseEvent->scenePos(), QTransform());
    if(item) printf("%d %d\n", (int)item->scenePos().x(), (int)item->scenePos().y());
  }
}
