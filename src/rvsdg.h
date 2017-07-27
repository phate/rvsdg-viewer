#ifndef RVSDG_H
#define RVSDG_H

#include <map>
#include <vector>
#include <string>
#include <QDomDocument>
#include <QAbstractItemModel>
#include <QModelIndex>

#define TAG_NODE "node"
#define TAG_REGION "region"
#define TAG_EDGE "edge"
#define TAG_INPUT "input"
#define TAG_OUTPUT "output"
#define TAG_ARGUMENT "argument"

#define ATTR_ID "id"
#define ATTR_SOURCE "source"
#define ATTR_TARGET "source"

///////////////////////////////////////////////////////////////////////////////

class Element {
  QString id;
  Element *parent;
  unsigned row;

  std::vector<Element*> children;
  std::vector<Element*> edges;

public:
  Element(QString id) {
    this->id = id;
    this->parent = NULL;
    this->row = 0;
  }
  Element(QString id, int row, Element *parent) {
    this->id = id;
    this->parent = parent;
    this->row = row;
  }
  void appendChild(Element *e) {
    children.insert(children.end(), e);
  }
  int numChildren() {
    return children.size();
  }
  Element *getParent() {
    return parent;
  }
  Element *getChild(unsigned n) {
    if(n < children.size()) return children[n];
    else return NULL;
  }
  unsigned getRow() {
    return row;
  }
  QString getId() {
    return id;
  }
  virtual QString getType() {
    return QString("");
  }
  void appendEdge(Element *e) {
    edges.insert(edges.end(), e);
  }
  int constructFromXml(const QDomElement &element, int row);
  virtual void insertElements(std::map<QString,Element*> &elements);

  void indent(int n) {
    for(int i = 0; i < n; i++) {
      printf(" ");
    }
  }
  void print(int n) {
    indent(n); printf("Element %s %d\n", id.toUtf8().constData(), row);
    for(unsigned i = 0; i < children.size(); i++) {
      children[i]->print(n+1);
    }
  }
};

///////////////////////////////////////////////////////////////////////////////

class Node : public Element {
  std::vector<Element*> inputs;
  std::vector<Element*> outputs;

public:
  Node(QString id, unsigned row, Element *parent) : Element(id, row, parent) {}
  QString getType() {
    return QString("Node");
  }
  void appendInput(Element *e) {
    inputs.insert(inputs.end(), e);
  }
  void appendOutput(Element *e) {
    outputs.insert(outputs.end(), e);
  }
  void insertElements(std::map<QString,Element*> &elements);
};

///////////////////////////////////////////////////////////////////////////////

class Input : public Element {
public:
  Input(QString id, Element *parent) : Element(id, 0, parent) {}
};

///////////////////////////////////////////////////////////////////////////////

class Output : public Element {
public:
  Output(QString id, Element *parent) : Element(id, 0, parent) {}
};

///////////////////////////////////////////////////////////////////////////////

class Region : public Element {
  std::vector<Element*> arguments;

public:
  Region(QString id, unsigned row, Element *parent) : Element(id, row, parent) {}
  QString getType() {
    return QString("Region");
  }
  void appendArgument(Element *e) {
    arguments.insert(arguments.end(), e);
  }
  void insertElements(std::map<QString,Element*> &elements);
};

///////////////////////////////////////////////////////////////////////////////

class Argument : public Element {
public:
  Argument(QString id, Element *parent) : Element(id, 0, parent) {}
};

///////////////////////////////////////////////////////////////////////////////

class Model : public QAbstractItemModel {
  Q_OBJECT

  std::map<QString,Element*> elements;
  Element *top;

public:  
  Model(QObject *parent = 0) : QAbstractItemModel(parent) {}
  ~Model() {}
  void constructFromXml(const QDomDocument &doc);
  void print() { top->print(0); }

  QModelIndex index(int row, int column, const QModelIndex &parent) const;
  QModelIndex parent(const QModelIndex &index) const;
  int rowCount(const QModelIndex &parent) const;
  int columnCount(const QModelIndex &parent) const;
  QVariant data(const QModelIndex &index, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;
};

#endif
