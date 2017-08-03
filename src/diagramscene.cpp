#include "diagramscene.h"
#include "rvsdg.h"

#include <QTextCursor>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsEllipseItem>

#define SPACING_X 50
#define SPACING_Y 50

#define LEFT_COLUMN 50
#define TOP_COLUMN 50

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
    for(auto node = region->results.begin(); node != region->results.end(); node++) {
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

  // find edge routing corridors
  std::vector<unsigned> currentRoutingXs(columnWidths.size()); // where multi-row edges are routed
  currentRoutingXs[0] = LEFT_COLUMN - LINE_CLEARANCE;
  int i = 1;
  for(auto width : columnWidths) {
    currentRoutingXs[i] = currentRoutingXs[i-1] + SPACING_X + width;
    i++;
  }

  // display vertices
  unsigned x = LEFT_COLUMN;
  unsigned y = TOP_COLUMN;
  unsigned maxHeight = 0;

  unsigned sceneWidth = 0;

  for(auto layer = layers.rbegin(); layer != layers.rend(); layer++) {
    int i = 0;
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

    x = LEFT_COLUMN;
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
          addLine(source->getX(), source->getY(), currentRoutingX, source->getY() + (SPACING_Y/2));
          addLine(currentRoutingX, source->getY() + (SPACING_Y/2), currentRoutingX, target->getY() - (SPACING_Y/2));
          addLine(currentRoutingX, target->getY() - (SPACING_Y/2), target->getX(), target->getY());

          currentRoutingXs[target->getColumn()] -= LINE_CLEARANCE;

        } else {
          addLine(source->getX(), source->getY(), target->getX(), target->getY());
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
