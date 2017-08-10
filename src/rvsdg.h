/******************************************************************************
 *
 * Classes representing the elements of the RVSDG graph
 *
 * Asbjørn Djupdal 2017
 *
 *****************************************************************************/

#ifndef RVSDG_H
#define RVSDG_H

#include <map>
#include <vector>
#include <string>
#include <QDomDocument>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QGraphicsPolygonItem>

#include "rvsdg-viewer.h"

///////////////////////////////////////////////////////////////////////////////

/* superclass of all RVSDG graph elements */
class Element {

protected:
  unsigned row;
  unsigned column;
  unsigned x;
  unsigned y;
  std::vector<QGraphicsLineItem*> lineSegments;

public:
  QString id;
  Element *parent;
  unsigned treeviewRow;
  std::vector<Element*> children;
  std::vector<Element*> edges;

  //---------------------------------------------------------------------------
  // constructors and destructor

  Element(QString id) {
    init(id);
  }
  Element(QString id, int treeviewRow, Element *parent) {
    init(id);
    this->parent = parent;
    this->treeviewRow = treeviewRow;
  }
  virtual ~Element() {
    for(auto it : children) {
      delete it;
    }
  }
  void init(QString id) {
    this->id = id;
    this->parent = NULL;
    this->treeviewRow = 0;
    row = column = x = y = 0;
  }

  //---------------------------------------------------------------------------
  // building the graph

  int constructFromXml(const QDomElement &element, int treeviewRow, std::map<QString,Element*> &elements);

  virtual Element *parseXmlElement(QString tagName, QString childId) {
    Q_UNUSED(tagName);
    Q_UNUSED(childId);
    Q_UNUSED(parent);
    return this;
  }

  void appendEdge(Element *e) {
    edges.insert(edges.end(), e);
  }

  void appendChild(Element *e) {
    children.insert(children.end(), e);
  }

  //---------------------------------------------------------------------------
  // graph information

  virtual Element *getVertex() {
    return this;
  }

  virtual unsigned getNumEdges() {
    return edges.size();
  }

  virtual Element *getEdge(unsigned n) {
    return edges[n];
  }

  virtual Element *getEdge(unsigned n, Element **source) {
    *source = this;
    return edges[n];
  }

  virtual void setLineSegments(unsigned n, std::vector<QGraphicsLineItem*>lines) {
    Q_UNUSED(n);
    lineSegments.insert(lineSegments.end(), lines.begin(), lines.end());
  }

  virtual void clearLineSegments() {
    lineSegments.clear();
  }

  virtual std::vector<QGraphicsLineItem*> getLineSegments() {
    return lineSegments;
  }

  virtual bool isSimpleNode() {
    return false;
  }

  virtual bool isComplexNode() {
    return false;
  }

  virtual QString getTypeName() {
    return QString("");
  }

  //---------------------------------------------------------------------------
  // graphical information, used when drawing

  virtual void setPos(unsigned x, unsigned y) {
    this->x = x;
    this->y = y;
  }
  void setRowCol(unsigned r, unsigned c) {
    row = r;
    column = c;
  }
  virtual unsigned getRow() {
    return row;
  }
  virtual unsigned getColumn() {
    return column;
  }
  virtual unsigned getX() {
    return x;
  }
  virtual unsigned getY() {
    return y;
  }
  virtual unsigned getWidth() {
    return 0;
  }
  virtual unsigned getHeight() {
    return 0;
  }
  /* appends QGraphicsItems representing this element to the given parent */
  virtual void appendItems(QGraphicsItem *parent) {
    Q_UNUSED(parent);
  }
};

///////////////////////////////////////////////////////////////////////////////

enum NodeType {
  NODE, LAMBDA, GAMMA, THETA, PHI
};

/* RVSDG node */
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

  ~Node() {
    for(auto it : inputs) {
      delete it;
    }
    for(auto it : outputs) {
      delete it;
    }
  }

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

  unsigned getNumEdges() {
    int totalSize = 0;
    for(auto it : outputs) {
      totalSize += it->getNumEdges();
    }
    return totalSize;
  }

  Element *getEdge(unsigned n) {
    return getEdge(n, NULL);
  }

  Element *getEdge(unsigned n, Element **source) {
    for(auto it : outputs) {
      if(n < it->getNumEdges()) {
        if(source) *source = it;
        return it->getEdge(n);
      }
      n -= it->getNumEdges();
    }
    return NULL;
  }

  void setLineSegments(unsigned n, std::vector<QGraphicsLineItem*>lines) {
    for(auto it : outputs) {
      if(n < it->getNumEdges()) {
        it->setLineSegments(n, lines);
      }
      n -= it->getNumEdges();
    }
  }

  void clearLineSegments() {
    for(auto it : outputs) {
      it->clearLineSegments();
    }
    for(auto it : inputs) {
      it->clearLineSegments();
    }
  }

  bool isSimpleNode() {
    if(children.size() == 0) return true;
    return false;
  }

  bool isComplexNode() {
    return !isSimpleNode();
  }

  QString getTypeName() {
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
};

///////////////////////////////////////////////////////////////////////////////

/* RVSDG node input */
class Input : public Element {
  QGraphicsPolygonItem *baseItem;

public:
  Input(QString id, Element *parent) : Element(id, 0, parent) {}
  Element *getVertex() {
    return parent;
  }
  unsigned getRow() {
    return parent->getRow();
  }
  unsigned getColumn() {
    return parent->getColumn();
  }
  unsigned getX() {
    return parent->getX() + x;
  }
  unsigned getY() {
    return parent->getY() + y;
  }
  unsigned getWidth() {
    return INPUTOUTPUT_SIZE;
  }
  unsigned getHeight() {
    return INPUTOUTPUT_SIZE;
  }
  void appendItems(QGraphicsItem *item);
  void setPos(unsigned x, unsigned y) {
    Element::setPos(x, y);
    baseItem->setPos(x, y);
  }
};

///////////////////////////////////////////////////////////////////////////////

/* RVSDG node output */
class Output : public Element {
  QGraphicsPolygonItem *baseItem;

public:
  Output(QString id, Element *parent) : Element(id, 0, parent) {}
  Element *getVertex() {
    return parent;
  }
  unsigned getRow() {
    return parent->getRow();
  }
  unsigned getColumn() {
    return parent->getColumn();
  }
  unsigned getX() {
    return parent->getX() + x;
  }
  unsigned getY() {
    return parent->getY() + y;
  }
  unsigned getWidth() {
    return INPUTOUTPUT_SIZE;
  }
  unsigned getHeight() {
    return INPUTOUTPUT_SIZE;
  }
  void appendItems(QGraphicsItem *item);
  void setPos(unsigned x, unsigned y) {
    Element::setPos(x, y);
    baseItem->setPos(x, y);
  }
};

///////////////////////////////////////////////////////////////////////////////

/* RVSDG region */
class Region : public Element {

  std::vector<std::vector<Element*>*> layers;
  unsigned width;
  unsigned height;

  void layer();

public:
  std::vector<Element*> arguments;
  std::vector<Element*> results;

  Region(QString id, unsigned treeviewRow, Element *parent) : Element(id, treeviewRow, parent) {}
  ~Region() {
    for(auto it : arguments) {
      delete it;
    }
    for(auto layer : layers) {
      delete layer;
    }
  }
  Element *parseXmlElement(QString tagName, QString childId);
  QString getTypeName() {
    return QString("Region");
  }
  void appendArgument(Element *e) {
    arguments.insert(arguments.end(), e);
  }
  void appendResult(Element *e) {
    results.insert(results.end(), e);
  }
  unsigned getWidth() {
    return width;
  }
  unsigned getHeight() {
    return height;
  }
  void appendItems(QGraphicsItem *item);
};

///////////////////////////////////////////////////////////////////////////////

/* RVSDG region argument */
class Argument : public Element {
  QGraphicsPolygonItem *baseItem;

public:
  Argument(QString id, Element *parent) : Element(id, 0, parent) {}
  unsigned getWidth() {
    return INPUTOUTPUT_SIZE;
  }
  unsigned getHeight() {
    return INPUTOUTPUT_SIZE;
  }
  void appendItems(QGraphicsItem *item);
  void setPos(unsigned x, unsigned y) {
    Element::setPos(x, y);
    baseItem->setPos(x, y);
  }
};

///////////////////////////////////////////////////////////////////////////////

/* RVSDG region result */
class Result : public Element {
  QGraphicsPolygonItem *baseItem;

public:
  Result(QString id, Element *parent) : Element(id, 0, parent) {}
  unsigned getWidth() {
    return INPUTOUTPUT_SIZE;
  }
  unsigned getHeight() {
    return INPUTOUTPUT_SIZE;
  }
  void appendItems(QGraphicsItem *item);
  void setPos(unsigned x, unsigned y) {
    Element::setPos(x, y);
    baseItem->setPos(x, y);
  }
};

#endif
