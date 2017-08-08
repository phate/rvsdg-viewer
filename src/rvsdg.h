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
#define ATTR_NAME "name"
#define ATTR_TYPE "type"

#define INPUTOUTPUT_SIZE 10
#define INPUTOUTPUT_CLEARANCE 10
#define TEXT_CLEARANCE 10
#define NODE_HEIGHT 100

#define SIMPLE_NODE_COLOR Qt::gray
#define GAMMA_NODE_COLOR  Qt::green
#define LAMBDA_NODE_COLOR Qt::blue
#define THETA_NODE_COLOR  Qt::red
#define PHI_NODE_COLOR    255,165,0

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
  virtual void setLineSegments(unsigned n, std::vector<QGraphicsLineItem*>lines) {
    Q_UNUSED(n);
    Q_UNUSED(lines);
  }
  virtual std::vector<QGraphicsLineItem*> getLineSegments() {
    return std::vector<QGraphicsLineItem*>(0);
  }
  virtual void clearLineSegments() {
  }
  virtual bool isRegion() {
    return false;
  }
  virtual bool isSimpleNode() {
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

enum NodeType {
  SIMPLE, LAMBDA, GAMMA, THETA, PHI
};

class Node : public Element {

  QString name;
  NodeType type;

public:
  std::vector<Element*> inputs;
  std::vector<Element*> outputs;

  // constructors and destructor

  Node(QString id, QString name, NodeType type, unsigned treeviewRow, Element *parent) : Element(id, treeviewRow, parent) {
    this->name = name;
    this->type = type;
  }

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

  QString getType() {
    return QString("Node");
  }

  bool isSimpleNode() {
    if(children.size() == 0) return true;
    return false;
  }

  // graphical information

  unsigned getWidth();

  unsigned addItem(DiagramScene *scene);
};

///////////////////////////////////////////////////////////////////////////////

class Input : public Element {

  std::vector<QGraphicsLineItem*> lineSegments;

public:
  Input(QString id, Element *parent) : Element(id, 0, parent) {}
  Element *getVertex() {
    return parent;
  }
  void setLineSegments(unsigned n, std::vector<QGraphicsLineItem*>lines) {
    Q_UNUSED(n);
    lineSegments = lines;
  }
  void clearLineSegments() {
    lineSegments.clear();
  }
  std::vector<QGraphicsLineItem*> getLineSegments() {
    return lineSegments;
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

  std::vector<QGraphicsLineItem*> lineSegments;

public:
  Output(QString id, Element *parent) : Element(id, 0, parent) {}
  Element *getVertex() {
    return parent;
  }
  void setLineSegments(unsigned n, std::vector<QGraphicsLineItem*>lines) {
    Q_UNUSED(n);
    lineSegments.insert(lineSegments.end(), lines.begin(), lines.end());
  }
  void clearLineSegments() {
    lineSegments.clear();
  }
  std::vector<QGraphicsLineItem*> getLineSegments() {
    return lineSegments;
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
  std::vector<QGraphicsLineItem*> lineSegments;

public:
  Argument(QString id, Element *parent) : Element(id, 0, parent) {}
  unsigned getWidth() {
    return INPUTOUTPUT_SIZE;
  }
  unsigned addItem(DiagramScene *scene);
  void setLineSegments(unsigned n, std::vector<QGraphicsLineItem*>lines) {
    Q_UNUSED(n);
    lineSegments.insert(lineSegments.end(), lines.begin(), lines.end());
  }
  std::vector<QGraphicsLineItem*> getLineSegments() {
    return lineSegments;
  }
  void clearLineSegments() {
    lineSegments.clear();
  }
};

///////////////////////////////////////////////////////////////////////////////

class Result : public Element {
  std::vector<QGraphicsLineItem*> lineSegments;

public:
  Result(QString id, Element *parent) : Element(id, 0, parent) {}
  void setLineSegments(unsigned n, std::vector<QGraphicsLineItem*>lines) {
    Q_UNUSED(n);
    lineSegments = lines;
  }
  std::vector<QGraphicsLineItem*> getLineSegments() {
    return lineSegments;
  }
  void clearLineSegments() {
    lineSegments.clear();
  }
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
