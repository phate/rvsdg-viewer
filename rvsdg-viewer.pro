QT += widgets

HEADERS       = src/mainwindow.h src/treeitem.h src/treemodel.h
SOURCES       = src/main.cpp src/mainwindow.cpp src/treeitem.cpp src/treemodel.cpp
RESOURCES     = application.qrc

# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/mainwindows/application
INSTALLS += target
