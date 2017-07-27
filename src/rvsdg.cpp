#include <stdio.h>
#include <QDebug>

#include "rvsdg.h"
#include <iostream>

void Model::constructFromXml(const QDomDocument &doc) {
  top = new Element("root");
  top->constructFromXml(doc.documentElement(), 0);
  top->insertElements(elements);

  QDomNodeList edgeList = doc.elementsByTagName(TAG_EDGE);

  for(int i = 0; i < edgeList.length(); i++) {
    QDomNode n = edgeList.at(i);
    QDomElement e = n.toElement();
    if(!e.isNull()) {
      QString source = e.attribute(ATTR_SOURCE);
      QString target = e.attribute(ATTR_TARGET);
      Element *sourceEl = elements.at(source);
      Element *targetEl = elements.at(target);
      // assert(sourceEl != NULL);
      // assert(targetEl != NULL);
      sourceEl->appendEdge(targetEl);
    }
  }
}

void Element::insertElements(std::map<QString,Element*> &elements) {
  elements[id] = this;
  for(auto it : children) {
    it->insertElements(elements);
  }
}

void Node::insertElements(std::map<QString,Element*> &elements) {
  Element::insertElements(elements);
  for(auto it : inputs) {
    it->insertElements(elements);
  }
  for(auto it : outputs) {
    it->insertElements(elements);
  }
}

void Region::insertElements(std::map<QString,Element*> &elements) {
  Element::insertElements(elements);
  for(auto it : arguments) {
    it->insertElements(elements);
  }
}

int Element::constructFromXml(const QDomElement &element, int row) {
  Element *child = this;

  if(element.tagName() == TAG_NODE) {
    child = new Node(element.attribute(ATTR_ID, ""), row++, this);
    appendChild(child);

  } else if(element.tagName() == TAG_REGION) {
    child = new Region(element.attribute(ATTR_ID, ""), row++, this);
    appendChild(child);

  } else if(element.tagName() == TAG_INPUT) {
    child = new Input(element.attribute(ATTR_ID, ""), this);
    ((Node*)this)->appendInput(child);

  } else if(element.tagName() == TAG_OUTPUT) {
    child = new Output(element.attribute(ATTR_ID, ""), this);
    ((Node*)this)->appendOutput(child);

  } else if(element.tagName() == TAG_ARGUMENT) {
    child = new Argument(element.attribute(ATTR_ID, ""), this);
    ((Region*)this)->appendArgument(child);
  }

  QDomNode n = element.firstChild();
  int i = 0;
  while(!n.isNull()) {
    QDomElement e = n.toElement();
    if(!e.isNull()) i = child->constructFromXml(e, i);
    n = n.nextSibling();
  }

  return row;
}

QModelIndex Model::index(int row, int column, const QModelIndex &parent) const {
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  Element *parentEl;

  if (!parent.isValid())
    parentEl = top;
  else
    parentEl = static_cast<Element*>(parent.internalPointer());

  Element *childEl = parentEl->getChild(row);

  if (childEl) return createIndex(row, column, childEl);
  else return QModelIndex();
}

QModelIndex Model::parent(const QModelIndex &index) const {
  if (!index.isValid())
    return QModelIndex();

  Element *childEl = static_cast<Element*>(index.internalPointer());
  Element *parentEl = childEl->getParent();

  if (parentEl == top) {
    return QModelIndex();
  }

  return createIndex(parentEl->getRow(), 0, parentEl);
}

int Model::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    return static_cast<Element*>(parent.internalPointer())->numChildren();
  } else {
    return top->numChildren();
  }
}

int Model::columnCount(const QModelIndex &parent) const {
  return 2;
}

QVariant Model::data(const QModelIndex &index, int role) const {
  if (!index.isValid())
    return QVariant();

  if (role != Qt::DisplayRole)
    return QVariant();

  Element *el = static_cast<Element*>(index.internalPointer());

  switch(index.column()) {
    case 0:
      return el->getType();
    case 1:
      return el->getId();
  }

  return QVariant();
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch(section) {
      case 0: return QVariant("Type"); break;
      case 1: return QVariant("Id"); break;
    }
  }
  return QVariant();
}

Qt::ItemFlags Model::flags(const QModelIndex &index) const {
  if (!index.isValid())
    return 0;

  return QAbstractItemModel::flags(index);
}
