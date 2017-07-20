#ifndef DIAGRAMSCENE_H
#define DIAGRAMSCENE_H

#include <QGraphicsScene>

class DiagramScene : public QGraphicsScene {
  Q_OBJECT

public:
  explicit DiagramScene(QObject *parent = 0);
};

#endif
