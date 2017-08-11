/******************************************************************************
 *
 * RVSDG region
 *
 * Asbjørn Djupdal 2017
 *
 *****************************************************************************/

#ifndef REGION_H
#define REGION_H

#include "element.h"

class Region : public Element {

  std::vector<std::vector<Element*>*> layers;
  unsigned width;
  unsigned height;

  void layer();

public:
  std::vector<Element*> arguments;
  std::vector<Element*> results;

  Region(QString id, unsigned treeviewRow, Element *parent) : Element(id, treeviewRow, parent) {}
  ~Region();
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

#endif
