/******************************************************************************
 *
 * RVSDG node
 *
 * Asbjørn Djupdal 2017
 *
 *****************************************************************************/

#ifndef NODE_H
#define NODE_H

#include "element.h"

enum NodeType {
  NODE, LAMBDA, GAMMA, THETA, PHI
};

class Node : public Element {

  unsigned width;
  unsigned height;
  QString name;
  NodeType type;
  QGraphicsPolygonItem *baseItem;
  bool expanded;

public:
  std::vector<Element*> inputs;
  std::vector<Element*> outputs;

  //---------------------------------------------------------------------------
  // constructors and destructor

  Node(QString id, QString name, NodeType type, unsigned treeviewRow, Element *parent) : Element(id, treeviewRow, parent) {
    this->name = name;
    this->type = type;
    width = 0;
    baseItem = NULL;
    expanded = false;
  }

  ~Node();

  //---------------------------------------------------------------------------
  // building the graph

  Element *parseXmlElement(QString tagName, QString childId);

  void appendInput(Element *e) {
    inputs.insert(inputs.end(), e);
  }
  void appendOutput(Element *e) {
    outputs.insert(outputs.end(), e);
  }

  //---------------------------------------------------------------------------
  // graph information

  unsigned getNumEdges();

  Edge *getEdge(unsigned n) {
    return getEdge(n, NULL);
  }

  Edge *getEdge(unsigned n, Element **source);

  void setLineSegments(unsigned n, std::vector<LineSegment>lines);

  bool isSimpleNode() {
    if(children.size() == 0) return true;
    return false;
  }

  bool isComplexNode() {
    return !isSimpleNode();
  }

  QString getTypeName();

  //---------------------------------------------------------------------------
  // graphical information, used when drawing

  void toggleExpanded() {
    expanded = !expanded;
  }

  void setPos(unsigned x, unsigned y) {
    Element::setPos(x, y);
    baseItem->setPos(x, y);
  }

  unsigned getWidth() {
    return width;
  }
  unsigned getHeight() {
    return height;
  }

  void appendItems(QGraphicsItem *item);

  virtual void clearColors() {
    Element::clearColors();
    for(auto child : inputs) {
      child->clearColors();
    }
    for(auto child : outputs) {
      child->clearColors();
    }
  }
};

#endif
