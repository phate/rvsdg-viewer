#include <stdio.h>
#include <QDebug>

#include "element.h"
#include "node.h"
#include "region.h"

#include <iostream>

Element::~Element() {
  for(auto it : children) {
    delete it;
  }
}

/* recursive function that constructs the graph from XML DOM elements */
/* the DOM element represents one child of this object */
int Element::constructFromXml(const QDomElement &element, int treeviewRow, std::map<QString,Element*> &elements) {
  Element *child = this;

  // create child from DOM element
  QString childId = element.attribute(ATTR_ID, "");
  QString tagName = element.tagName();

  if(tagName == TAG_NODE) {
    QString childName = element.attribute(ATTR_NAME, "");
    QString childTypeS = element.attribute(ATTR_TYPE, "");

    NodeType childType = NODE;
    if(childTypeS == "lambda") childType = LAMBDA;
    else if(childTypeS == "gamma") childType = GAMMA;
    else if(childTypeS == "theta") childType = THETA;
    else if(childTypeS == "phi") childType = PHI;

    child = new Node(childId, childName, childType, treeviewRow++, this);
    appendChild(child);

  } else if(tagName == TAG_REGION) {
    child = new Region(childId, treeviewRow++, this);
    appendChild(child);

  } else {
    child = parseXmlElement(tagName, childId);
  }

  // append child to children vector
  if(child != this) elements[childId] = child;

  // loop through all the DOM element children and construct granchildren recursivly
  QDomNode n = element.firstChild();
  int i = 0;
  while(!n.isNull()) {
    QDomElement e = n.toElement();
    if(!e.isNull()) i = child->constructFromXml(e, i, elements);
    n = n.nextSibling();
  }

  return treeviewRow;
}
