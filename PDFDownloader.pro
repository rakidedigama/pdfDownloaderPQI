#-------------------------------------------------
#
# Project created by QtCreator 2017-05-15T10:50:17
#
#-------------------------------------------------

QT       +=  core xml network xmlpatterns

QT       -= gui

TARGET = PDFDownloader
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH += ../../CPPCommon

include(../../../PQIS_ProjectIncludes/boost.pri)
include(../../../PQIS_ProjectIncludes/messaginglibs.pri)
include(../../../PQIS_ProjectIncludes/boost.pri)
include(../../../PQIS_ProjectIncludes/jsonlib.pri)
include(../../../PQIS_ProjectIncludes/qtftp.pri)
include(../../../PQIS_ProjectIncludes/Tiffio.pri)

SOURCES += main.cpp \
    ../../CPPCommon/IniFile.cpp \
    ../../CPPCommon/Logger.cpp \
    ../../CPPCommon/LTime.cpp \
    ../../CPPCommon/MQTopicInterface.cpp \
../../CPPCommon/huffman.cpp \
    FTPpusher.cpp\
    pdfconverter.cpp\
    Tiffconverter.cpp

HEADERS += "FTPpusher.h" \
           "pdfconverter.h" \
            "Tiffconverter.h"






OTHER_FILES += \
    ../../../PQIS_ProjectIncludes/qtftp.txt
