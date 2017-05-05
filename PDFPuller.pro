#-------------------------------------------------
#
# Project created by QtCreator 2016-10-31T10:03:11
#
#-------------------------------------------------

QT       += core xml network xmlpatterns

QT       -= gui

DEFINES += WIN32_LEAN_AND_MEAN

TARGET = PDFPuller
CONFIG   += console
CONFIG   -= app_bundle


#CONFIG(debug, debug|release){
 #   LIBS += -lQt5Ftpd
#} else {
#    LIBS += -lQt5Ftp
#}

INCLUDEPATH += ../../../CPPCommon

include(../../../../PQIS_ProjectIncludes/boost.pri)
include(../../../../PQIS_ProjectIncludes/messaginglibs.pri)
include(../../../../PQIS_ProjectIncludes/boost.pri)
include(../../../../PQIS_ProjectIncludes/jsonlib.pri)
include(../../../../PQIS_ProjectIncludes/qtftp.pri)



TEMPLATE = app


SOURCES += main.cpp \
    ftpfilesniffer.cpp \
    FTPpusher.cpp \
    ../../../CPPCommon/IniFile.cpp \
    ../../../CPPCommon/Logger.cpp \
    ../../../CPPCommon/LTime.cpp \
    ../../../CPPCommon/MQTopicInterface.cpp \
    xmlmsghandler.cpp

HEADERS += \
    ftpfilesniffer.h \
    FTPpusher.h \
    xmlmsghandler.h

OTHER_FILES += \
    ../../../../PQIS_ProjectIncludes/qtftp.txt
