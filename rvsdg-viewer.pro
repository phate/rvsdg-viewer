QT += widgets xml
QMAKE_CXXFLAGS += -g -std=gnu++11

HEADERS       = src/mainwindow.h src/diagramscene.h src/diagramview.h src/model.h src/rvsdg-viewer.h src/element.h src/node.h src/region.h src/input.h src/output.h src/argument.h src/result.h
SOURCES       = src/rvsdg-viewer.cpp src/mainwindow.cpp src/diagramscene.cpp src/model.cpp src/element.cpp src/node.cpp src/region.cpp
RESOURCES     = application.qrc

# install
target.path = /usr/bin/
INSTALLS += target
