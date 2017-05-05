#-------------------------------------------------
#
# Project created by QtCreator 2016-10-31T10:03:11
#
#-------------------------------------------------

QT       += core xml network xmlpatterns

QT       -= gui

DEFINES += WIN32_LEAN_AND_MEAN

TARGET = SeittiXMLService
CONFIG   += console
CONFIG   -= app_bundle

INCLUDEPATH += ../../CPPCommon ../../QTCommon

include(../../../SE_ProjectIncludes/boost.pri)
include(../../../SE_ProjectIncludes/MongoClient.pri)
include(../../../SE_ProjectIncludes/messaginglibs.pri)
include(../../../SE_ProjectIncludes/jsonlib.pri)


TEMPLATE = app


SOURCES += main.cpp \
    ftpfilesniffer.cpp \
    FTPpusher.cpp \
    ../../CPPCommon/IniFile.cpp \
    ../../CPPCommon/Logger.cpp \
    ../../CPPCommon/LTime.cpp \
    ../../CPPCommon/MQTopicInterface.cpp \
    xmlmsghandler.cpp

HEADERS += \
    ftpfilesniffer.h \
    FTPpusher.h \
    xmlmsghandler.h
