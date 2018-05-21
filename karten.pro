TEMPLATE = app
LANGUAGE = C++
QT += qt3support core gui xml
LIBS += -lkdeui
CONFIG += qt \
    warn_on
#    release
HEADERS += ukartendownload.h \
    region.h \
    Atlas.h \
    mapservermanager.h \
    koordinateneingabe.h \
    waypointdialog.h \
    kompass.h \
    kompassinterface.h \
    track.h \
    upopupmenu.h \
    settingdialog.h \
    timedtrackpoint.h \
    timedtrack.h \
    timedtrackdialog.h \
    tracktimer.h \
    hoehenprofil.h \
    hoehenabfrager.h \
    leistungsprofil.h \
    cartesian.h
SOURCES += main.cpp \
    Atlas.cpp \
    ukartendownload.cpp \
    region.cpp \
    mapservermanager.cpp \
    koordinateneingabe.cpp \
    waypointdialog.cpp \
    kompass.cpp \
    kompassinterface.cpp \
    track.cpp \
    upopupmenu.cpp \
    settingdialog.cpp \
    timedtrackpoint.cpp \
    timedtrack.cpp \
    timedtrackdialog.cpp \
    tracktimer.cpp \
    hoehenprofil.cpp \
    hoehenabfrager.cpp \
    leistungsprofil.cpp \
    cartesian.cpp
FORMS = kartendownload_qt4.ui \
    mapserverdialog.ui \
    waypointdialog.ui \
    koordinateneingabe.ui \
    kompass.ui \
    trackdialog.ui \
    settingdialog.ui \
    changeUserAgent.ui \
    timedtrackdialog.ui \
    tracktimer.ui \
    hoehendialog.ui
LIBS += ../terminal/terminallog.o ../terminal/moc_terminallog.o ../Karten/GoogleElevationAPI/src/polylineencoder.o
INCLUDEPATH += "../terminal" "./GoogleElevationAPI/src"

# The following line was inserted by qt3to4
# CONFIG += uic
QMAKE_CXXFLAGS += -std=c++14
RESOURCES += \
    bilder.qrc \
    hilfeseiten.qrc
