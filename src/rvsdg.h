#ifndef RVSDG_H
#define RVSDG_H

#include <map>
#include <vector>
#include <string>
#include <QDomDocument>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QGraphicsPolygonItem>

#define TAG_NODE "node"
#define TAG_REGION "region"
#define TAG_EDGE "edge"
#define TAG_INPUT "input"
#define TAG_OUTPUT "output"
#define TAG_ARGUMENT "argument"
#define TAG_RESULT "result"

#define ATTR_ID "id"
#define ATTR_SOURCE "source"
#define ATTR_TARGET "target"

#define INPUTOUTPUT_SIZE 10
#define INPUTOUTPUT_CLEARANCE 10
#define TEXT_CLEARANCE 10
#define NODE_HEIGHT 100

class DiagramScene;

///////////////////////////////////////////////////////////////////////////////

class Element {

protected:
  unsigned row;
  unsigned column;
  unsigned x;
  unsigned y;

public:
  QString id;
  Element *parent;
  unsigned treeviewRow;
  std::vector<Element*> children;
  std::vector<Element*> edges;

  // constructors and destructor

  Element(QString id) {
    this->id = id;
    this->parent = NULL;
    this->treeviewRow = 0;
    row = column = x = y = 0;
  }
  Element(QString id, int treeviewRow, Element *parent) {
    this->id = id;
    this->parent = parent;
    this->treeviewRow = treeviewRow;
    row = column = x = y = 0;
  }
  virtual ~Element() {
    for(auto it : children) {
      delete it;
    }
  }

  // constructing the graph

  int constructFromXml(const QDomElement &element, int treeviewRow, std::map<QString,Element*> &elements);

  void appendEdge(Element *e) {
    edges.insert(edges.end(), e);
  }

  void appendChild(Element *e) {
    children.insert(children.end(), e);
  }

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
  virtual bool isRegion() {
    return false;
  }
  virtual QString getType() {
    return QString("");
  }

  // graphical information

  void setPos(unsigned x, unsigned y) {
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
  virtual unsigned addItem(DiagramScene *scene) {
    Q_UNUSED(scene);
    return 0;
  }
};

///////////////////////////////////////////////////////////////////////////////

class Node : public Element {

public:
  std::vector<Element*> inputs;
  std::vector<Element*> outputs;

  // constructors and destructor

  Node(QString id, unsigned treeviewRow, Element *parent) : Element(id, treeviewRow, parent) {}

  ~Node() {
    for(auto it : inputs) {
      delete it;
    }
    for(auto it : outputs) {
      delete it;
    }
  }

  // constructing the graph

  void appendInput(Element *e) {
    e->setPos(inputs.size() * (INPUTOUTPUT_SIZE + INPUTOUTPUT_CLEARANCE) + INPUTOUTPUT_CLEARANCE, 0);
    inputs.insert(inputs.end(), e);
  }
  void appendOutput(Element *e) {
    e->setPos(outputs.size() * (INPUTOUTPUT_SIZE + INPUTOUTPUT_CLEARANCE) + INPUTOUTPUT_CLEARANCE + INPUTOUTPUT_CLEARANCE/2, NODE_HEIGHT);
    outputs.insert(outputs.end(), e);
  }

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

  QString getType() {
    return QString("Node");
  }

  // graphical information

  unsigned getWidth();

  unsigned addItem(DiagramScene *scene);
};

///////////////////////////////////////////////////////////////////////////////

class Input : public Element {

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
};

///////////////////////////////////////////////////////////////////////////////

class Output : public Element {

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
};

///////////////////////////////////////////////////////////////////////////////

class Region : public Element {

public:
  std::vector<Element*> arguments;
  std::vector<Element*> results;

  Region(QString id, unsigned treeviewRow, Element *parent) : Element(id, treeviewRow, parent) {}
  ~Region() {
    for(auto it : arguments) {
      delete it;
    }
  }
  bool isRegion() { return true; }
  QString getType() {
    return QString("Region");
  }
  void appendArgument(Element *e) {
    arguments.insert(arguments.end(), e);
  }
  void appendResult(Element *e) {
    results.insert(results.end(), e);
  }
};

///////////////////////////////////////////////////////////////////////////////

class Argument : public Element {
public:
  Argument(QString id, Element *parent) : Element(id, 0, parent) {}
  unsigned getWidth() {
    return INPUTOUTPUT_SIZE;
  }
  unsigned addItem(DiagramScene *scene);
};

///////////////////////////////////////////////////////////////////////////////

class Result : public Element {
public:
  Result(QString id, Element *parent) : Element(id, 0, parent) {}
  unsigned getWidth() {
    return INPUTOUTPUT_SIZE;
  }
  unsigned addItem(DiagramScene *scene);
};

///////////////////////////////////////////////////////////////////////////////

class Model : public QAbstractItemModel {
  Q_OBJECT

  std::map<QString,Element*> elements;
  Element *top;

public:  
  Model(const QDomDocument &doc, QObject *parent = 0);
  ~Model() {
    delete top;
  }

  QModelIndex index(int treeviewRow, int column, const QModelIndex &parent) const;
  QModelIndex parent(const QModelIndex &index) const;
  int rowCount(const QModelIndex &parent) const;
  int columnCount(const QModelIndex &parent) const;
  QVariant data(const QModelIndex &index, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;
};

#endif
