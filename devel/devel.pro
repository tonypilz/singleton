TEMPLATE = app
CONFIG += c++11
CONFIG -= app_bundle
QT += testlib

#QMAKE_CXXFLAGS += -E

include($$PWD/src/src.pri)
include($$PWD/tests/tests.pri)
include($$PWD/examples/examples.pri)

SOURCES += \
    main.cpp 

HEADERS += \

