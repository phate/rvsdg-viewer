#ifndef EDGE_H
#define EDGE_H

#include "element.h"

class Element;

class Edge {
public:
  int color;
  Element *target;
  unsigned zvalue;

  Edge(Element *target) {
    this->target = target;
    color = -1;
  }
};

#endif
