QT += widgets xml
QMAKE_CXXFLAGS += -g

HEADERS       = src/mainwindow.h src/treeitem.h src/treemodel.h src/diagramscene.h
SOURCES       = src/rvsdg-viewer.cpp src/mainwindow.cpp src/treeitem.cpp src/treemodel.cpp src/diagramscene.cpp
RESOURCES     = application.qrc

# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/mainwindows/application
INSTALLS += target
