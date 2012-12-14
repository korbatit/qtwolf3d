QT        += testlib

TARGET     = tst_page
CONFIG    -= app_bundle

TEMPLATE   = app

HEADERS   += ../../../wlpage.h \
    ../../../wlpalette.h \
    ../../../renderer.h

SOURCES   += tst_page.cpp ../../../wlpage.cpp \
    ../../../wlpalette.cpp

RESOURCES += page.qrc


