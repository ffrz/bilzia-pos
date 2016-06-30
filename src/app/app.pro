include(../global.pri)

QT = core gui network widgets sql printsupport
TARGET = bilzia-pos
TEMPLATE = app
DESTDIR = $$PWD/../../dist
RC_FILE += app.rc
SOURCES += \
    main.cpp\
    mainwindow.cpp \
    sales/salesordermanager.cpp \
    sales/salesordermodel.cpp \
    sales/salesorderproxymodel.cpp \
    sales/salesordereditor.cpp \
    sales/salesordereditorproductmodel.cpp

HEADERS  += \
    mainwindow.h \
    sales/salesordermanager.h \
    sales/salesordermodel.h \
    sales/salesorderproxymodel.h \
    sales/salesordereditor.h \
    sales/salesordereditorproductmodel.h
