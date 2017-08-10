QT += widgets xml
QMAKE_CXXFLAGS += -g -std=gnu++11

HEADERS       = src/mainwindow.h src/diagramscene.h src/rvsdg.h src/diagramview.h src/model.h src/rvsdg-viewer.h
SOURCES       = src/rvsdg-viewer.cpp src/mainwindow.cpp src/diagramscene.cpp src/rvsdg.cpp src/model.cpp
RESOURCES     = application.qrc

# install
target.path = /usr/bin/
INSTALLS += target
