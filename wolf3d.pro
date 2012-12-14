QT += multimedia core gui

#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = wolf3d

#DEFINES   += TOUCHSCREEN
#CONFIG    += mobility
#MOBILITY  += sensor
#LIBS      += -lQtSensors

symbian {
    DEFINES   += TOUCHSCREEN
    CONFIG    += mobility
    MOBILITY  += sensor
    VERSION    = 1.0.0

    OBJECTS_DIR = build
    MOC_DIR     = build
    UI_DIR      = build

    DESTDIR     = build
    TEMPLATE    = app
    #CONFIG      += debug

    QT += svg
    DEFINES += SYMBIAN_PLATFORM
    TARGET.UID3 = 0xe1212120
    TARGET.CAPABILITY +=
    TARGET.EPOCSTACKSIZE = 0x14000
    TARGET.EPOCHEAPSIZE = 0x020000 0x800000
    ICON = symbian/wolf3d.svg

    #demoFiles.sources = audiot.wl1 audiohed.wl1 map vswap.wl1 gamemaps.wl1 maphead.wl1 vgadict.wl1 vgahead.wl1 vgagraph.wl1
    #DEPLOYMENT += demoFiles
}

HEADERS += fmopl.h foreign.h version.h wl_def.h wl_menu.h sinctable.h sound.h main.h display.h \
           wlpage.h wlcache.h wlaudio.h wltext.h wlinput.h wlstate.h wlitem.h wlplay.h resample.h wlrender.h \
           wlpaint.h wluser.h \
    audio.h

SOURCES += fmopl.cpp signon.cpp wl_act2.cpp wl_agent.cpp wl_draw.cpp wl_game.cpp wl_inter.cpp wl_menu.cpp \
           sound.cpp main.cpp display.cpp wlpage.cpp wlpalette.cpp wlcache.cpp wlaudio.cpp wltext.cpp \
           wlinput.cpp wlstate.cpp wlitem.cpp wlplay.cpp resample.cpp wlrender.cpp wlpaint.cpp \
           wluser.cpp

maemo5 {
    DEFINES   += TOUCHSCREEN
    CONFIG    += mobility
    MOBILITY  += sensor
    LIBS      += -lQtSensors

    INSTALLS    += target
    target.path = /opt/usr/bin

    INSTALLS    += desktop
    desktop.path  = /usr/share/applications/hildon
    desktop.files  = data/wolf3d.desktop

    INSTALLS    += service
    service.path  = /usr/share/dbus-1/services
    service.files  = data/wolf3d.service

    INSTALLS    += icon64
    icon64.path  = /usr/share/icons/hicolor/64x64/apps
    icon64.files  = data/64x64/wolf3d.png

    INSTALLS    += icon48
    icon48.path  = /usr/share/icons/hicolor/48x48/apps
    icon64.files  = data/48x48/wolf3d.png

    INSTALLS    += content
    content.path = /opt/usr/share/wolf3d
    content.files = *.wl6

    INSTALLS    += launcher
    launcher.path = /opt/usr/bin
    launcher.files = data/wolf3d.sh
}

OTHER_FILES += \
    qtc_packaging/debian_harmattan/rules \
    qtc_packaging/debian_harmattan/README \
    qtc_packaging/debian_harmattan/manifest.aegis \
    qtc_packaging/debian_harmattan/copyright \
    qtc_packaging/debian_harmattan/control \
    qtc_packaging/debian_harmattan/compat \
    qtc_packaging/debian_harmattan/changelog \
    qtc_packaging/debian_fremantle/rules \
    qtc_packaging/debian_fremantle/README \
    qtc_packaging/debian_fremantle/copyright \
    qtc_packaging/debian_fremantle/control \
    qtc_packaging/debian_fremantle/compat \
    qtc_packaging/debian_fremantle/changelog

