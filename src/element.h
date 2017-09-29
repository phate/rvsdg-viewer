/******************************************************************************
 *
 * Superclass of all RVSDG graph elements
 *
 * Asbjørn Djupdal 2017
 *
 *****************************************************************************/

#ifndef ELEMENT_H
#define ELEMENT_H

#include <map>
#include <vector>
#include <string>
#include <QDomDocument>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QGraphicsPolygonItem>

#include "rvsdg-viewer.h"
#include "linesegment.h"

class Edge;

class Element {

protected:
  unsigned row;
  unsigned column;
  unsigned x;
  unsigned y;
  std::vector<LineSegment> lineSegments;

  void init(QString id) {
    this->id = id;
    this->parent = NULL;
    this->treeviewRow = 0;
    row = column = x = y = 0;
  }

public:
  QString id;
  Element *parent;
  unsigned treeviewRow;
  std::vector<Element*> children;
  std::vector<Edge*> edges;

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
  virtual ~Element();

  //---------------------------------------------------------------------------
  // building the graph

  int constructFromXml(const QDomElement &element, int treeviewRow, std::map<QString,Element*> &elements);

  virtual Element *parseXmlElement(QString tagName, QString childId) {
    Q_UNUSED(tagName);
    Q_UNUSED(childId);
    return this;
  }

  void appendEdge(Edge *e) {
    edges.push_back(e);
  }

  void appendChild(Element *e) {
    children.push_back(e);
  }

  //---------------------------------------------------------------------------
  // graph information

  virtual Element *getVertex() {
    return this;
  }

  virtual unsigned getNumEdges() {
    return edges.size();
  }

  virtual Edge *getEdge(unsigned n) {
    return edges[n];
  }

  virtual Edge *getEdge(unsigned n, Element **source) {
    *source = this;
    return edges[n];
  }

  virtual void setLineSegments(unsigned n, std::vector<LineSegment>lines) {
    Q_UNUSED(n);
    lineSegments.insert(lineSegments.end(), lines.begin(), lines.end());
  }

  virtual std::vector<LineSegment> getLineSegments() {
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
  virtual void clearLineSegments() {
    lineSegments.clear();
  }
  virtual void clearColors() {
    for(auto edge : edges) {
      edge->color = -1;
    }
    for(auto child : children) {
      child->clearColors();
    }
  }
};

#endif

