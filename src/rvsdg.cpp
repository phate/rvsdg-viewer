#include <stdio.h>
#include <QDebug>

#include "rvsdg.h"
#include <iostream>

#include "diagramscene.h"

///////////////////////////////////////////////////////////////////////////////

int Element::constructFromXml(const QDomElement &element, int treeviewRow, std::map<QString,Element*> &elements) {
  Element *child = this;

  QString childId = element.attribute(ATTR_ID, "");

  // FIXME: dangerous casting, will segfault with invalid XML
  if(element.tagName() == TAG_NODE) {
    child = new Node(childId, treeviewRow++, this);
    appendChild(child);

  } else if(element.tagName() == TAG_REGION) {
    child = new Region(childId, treeviewRow++, this);
    appendChild(child);

  } else if(element.tagName() == TAG_INPUT) {
    child = new Input(childId, this);
    ((Node*)this)->appendInput(child);

  } else if(element.tagName() == TAG_OUTPUT) {
    child = new Output(childId, this);
    ((Node*)this)->appendOutput(child);

  } else if(element.tagName() == TAG_ARGUMENT) {
    child = new Argument(childId, this);
    ((Region*)this)->appendArgument(child);

  } else if(element.tagName() == TAG_RESULT) {
    child = new Result(childId, this);
    ((Region*)this)->appendResult(child);
  }

  if(child != this) elements[childId] = child;

  QDomNode n = element.firstChild();
  int i = 0;
  while(!n.isNull()) {
    QDomElement e = n.toElement();
    if(!e.isNull()) i = child->constructFromXml(e, i, elements);
    n = n.nextSibling();
  }

  return treeviewRow;
}

///////////////////////////////////////////////////////////////////////////////

unsigned Node::getWidth() {
  unsigned width = inputs.size() * (INPUTOUTPUT_CLEARANCE + INPUTOUTPUT_SIZE);
  unsigned outputWidth = outputs.size() * (INPUTOUTPUT_CLEARANCE + INPUTOUTPUT_SIZE);

  if(outputWidth > width) width = outputWidth;

  // FIXME: find a way to avoid creating a dummy textitem just for the size
  QGraphicsTextItem *text = new QGraphicsTextItem(id);
  unsigned textwidth = text->boundingRect().width() + TEXT_CLEARANCE*2;
  if(textwidth > width) width = textwidth;
  delete text;

  return width;
}

QGraphicsItem *Node::getItem(unsigned x, unsigned y) {
  GfxNode *node = new GfxNode(id, getWidth());

  int xx = INPUTOUTPUT_CLEARANCE;

  for(auto input : inputs) {
    Q_UNUSED(input);
    GfxInput *inputItem = new GfxInput(node);
    inputItem->setPos(QPointF(xx, 0));
    xx += INPUTOUTPUT_CLEARANCE + INPUTOUTPUT_SIZE;
  }

  xx = INPUTOUTPUT_CLEARANCE;

  for(auto output : outputs) {
    Q_UNUSED(output);
    GfxOutput *outputItem = new GfxOutput(node);
    outputItem->setPos(QPointF(xx, 100));
    xx += INPUTOUTPUT_CLEARANCE + INPUTOUTPUT_SIZE;
  }

  node->setPos(QPointF(x, y));

  return node;
}

///////////////////////////////////////////////////////////////////////////////

QGraphicsItem *Argument::getItem(unsigned x, unsigned y) {
  GfxArgument *argument = new GfxArgument;
  argument->setPos(QPointF(x, y));
  return argument;
}

///////////////////////////////////////////////////////////////////////////////

QGraphicsItem *Result::getItem(unsigned x, unsigned y) {
  GfxResult *result = new GfxResult;
  result->setPos(QPointF(x, y));
  return result;
}

///////////////////////////////////////////////////////////////////////////////

QModelIndex Model::index(int treeviewRow, int column, const QModelIndex &parent) const {
  if (!hasIndex(treeviewRow, column, parent))
    return QModelIndex();

  Element *parentEl;

  if (!parent.isValid())
    parentEl = top;
  else
    parentEl = static_cast<Element*>(parent.internalPointer());

  Element *childEl = parentEl->children[treeviewRow];

  if (childEl) return createIndex(treeviewRow, column, childEl);
  else return QModelIndex();
}

QModelIndex Model::parent(const QModelIndex &index) const {
  if (!index.isValid())
    return QModelIndex();

  Element *childEl = static_cast<Element*>(index.internalPointer());
  Element *parentEl = childEl->parent;

  if (parentEl == top) {
    return QModelIndex();
  }

  return createIndex(parentEl->treeviewRow, 0, parentEl);
}

int Model::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    return static_cast<Element*>(parent.internalPointer())->children.size();
  } else {
    return top->children.size();
  }
}

int Model::columnCount(const QModelIndex &parent) const {
  Q_UNUSED(parent);
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
      return el->id;
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

Model::Model(const QDomDocument &doc, QObject *parent) : QAbstractItemModel(parent) {
  top = new Element("root");
  top->constructFromXml(doc.documentElement(), 0, elements);

  QDomNodeList edgeList = doc.elementsByTagName(TAG_EDGE);

  for(int i = 0; i < edgeList.length(); i++) {
    QDomNode n = edgeList.at(i);
    QDomElement e = n.toElement();
    if(!e.isNull()) {
      QString source = e.attribute(ATTR_SOURCE);
      QString target = e.attribute(ATTR_TARGET);
      Element *sourceEl = elements.at(source);
      Element *targetEl = elements.at(target);

      if((sourceEl == NULL) || (targetEl == NULL)) {
        delete top;
        throw std::exception();
      }
      sourceEl->appendEdge(targetEl);
    }
  }
}
