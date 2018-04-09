TEMPLATE = app
CONFIG += c++11
CONFIG -= app_bundle
QT += testlib
CONFIG += testcase

QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS += -Wpedantic
QMAKE_CXXFLAGS += -fno-rtti
#QMAKE_CXXFLAGS += -fno-exceptions

include($$PWD/src/src.pri)
include($$PWD/tests/tests.pri)
include($$PWD/examples/examples.pri)

#DEFINES += USE_SINGLE_HEADER

SOURCES += \
    main.cpp 

HEADERS += \

