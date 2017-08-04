QT += widgets xml
QMAKE_CXXFLAGS += -g -std=gnu++11

HEADERS       = src/mainwindow.h src/diagramscene.h src/rvsdg.h src/diagramview.h
SOURCES       = src/rvsdg-viewer.cpp src/mainwindow.cpp src/diagramscene.cpp src/rvsdg.cpp
RESOURCES     = application.qrc

# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/mainwindows/application
INSTALLS += target
