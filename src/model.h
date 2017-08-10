/******************************************************************************
 *
 * Qt model used for holding data for the QTreeView
 *
 * Asbjørn Djupdal 2017
 *
 *****************************************************************************/

#ifndef MODEL_H
#define MODEL_H

#include <map>
#include <vector>
#include <string>
#include <QDomDocument>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QGraphicsPolygonItem>

#include "rvsdg.h"

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
