#include "model.h"
#include "edge.h"

QModelIndex Model::index(int treeviewRow, int column, const QModelIndex &parent) const {
  if (!hasIndex(treeviewRow, column, parent))
    return QModelIndex();

  Element *parentEl;

  if (!parent.isValid())
    parentEl = top;
  else
    parentEl = static_cast<Element*>(parent.internalPointer());

  Element *childEl = NULL;

  int n = 0;
  for(auto it : parentEl->children) {
    if(!it->isSimpleNode()) {
      if(treeviewRow == n) {
        childEl = it;
        break;
      }
      n++;
    }
  }

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
    unsigned n = 0;
    for(auto child : static_cast<Element*>(parent.internalPointer())->children) {
      if(!child->isSimpleNode()) n++;
    }
    return n;
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
      return el->getTypeName();
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
      sourceEl->appendEdge(new Edge(targetEl));
    }
  }
}

void Model::clearColors() {
  top->clearColors();
}
