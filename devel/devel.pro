TEMPLATE = app
CONFIG += c++11
CONFIG -= app_bundle
QT += testlib

include($$PWD/src/src.pri)
include($$PWD/tests/tests.pri)
include($$PWD/examples/examples.pri)

#DEFINES += USE_SINGLE_HEADER

SOURCES += \
    main.cpp 

HEADERS += \

