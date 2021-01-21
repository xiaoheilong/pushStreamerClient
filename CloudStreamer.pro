#-------------------------------------------------
#
# Project created by QtCreator 2020-12-11T13:54:13
#
#-------------------------------------------------

QT       += core gui
QT       += network
QT       += concurrent
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CloudStreamer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
#cloudGame
greaterThan(QT_MAJOR_VERSION, 4) {
    TARGET_ARCH=$${QT_ARCH}
} else {
    TARGET_ARCH=$${QMAKE_HOST.arch}
}


INCLUDEPATH += $$PWD/./cloudGame/include



#加入调试信息
QMAKE_CFLAGS_RELEASE += -g
QMAKE_CXXFLAGS_RELEASE += -g
#禁止优化
QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE -= -O2
#release在最后link时默认有"-s”参数，表示"Omit all symbol information from the output file"，因此要去掉该参
win32:!msvc{
    QMAKE_LFLAGS_RELEASE = -mthreads
}else{
    #message("msvc version")
    QMAKE_CFLAGS_RELEASE = -O2 -MD -Zi
    QMAKE_LFLAGS_RELEASE = /INCREMENTAL:NO /DEBUG
    QMAKE_LFLAGS_RELEASE += /MAP
    QMAKE_CFLAGS_RELEASE += /Zi
    QMAKE_LFLAGS_RELEASE += /debug /opt:ref
    QMAKE_LFLAGS_RELEASE += /MAP /DEBUG /opt:ref /INCREMENTAL:NO
}


SOURCES += \
        cloudGame/include/easywsclient.cpp \
        cloudGame/include/lib_json/json_reader.cpp \
        cloudGame/include/lib_json/json_value.cpp \
        cloudGame/include/lib_json/json_writer.cpp \
        main.cpp \
        cloudstreamer.cpp \
    wsservice.cpp \
    cloudgameserviceiterator.cpp \
    dealinifile.cpp \
    wsservicecloudgame.cpp \
    wsservicecloudgamethread.cpp \
    cloudgaestreamdeal.cpp \
    keyvaluetransformt.cpp \
    gamecommandexetransformt.cpp \
    consumerkeyboardvalue.cpp

HEADERS += \
        cloudGame/include/easywsclient.hpp \
        cloudGame/include/json/autolink.h \
        cloudGame/include/json/config.h \
        cloudGame/include/json/features.h \
        cloudGame/include/json/forwards.h \
        cloudGame/include/json/json.h \
        cloudGame/include/json/reader.h \
        cloudGame/include/json/value.h \
        cloudGame/include/json/writer.h \
        cloudGame/include/lib_json/json_batchallocator.h \
        cloudGame/include/lib_json/json_internalarray.inl \
        cloudGame/include/lib_json/json_internalmap.inl \
        cloudGame/include/lib_json/json_valueiterator.inl \
        cloudstreamer.h \
    wsservice.h \
    cloudgameserviceiterator.h \
    dealinifile.h \
    wsservicecloudgame.h \
    wsservicecloudgamethread.h \
    cloudgaestreamdeal.h \
    cloudGame/include/CloudGame.h \
    keyvaluetransformt.h \
    gamecommandexetransformt.h \
    consumerkeyboardvalue.h

FORMS += \
        cloudstreamer.ui \
   # cloudstreamerKeyBorad.ui \
   # cloudstreamerStreamer.ui

INCLUDEPATH += $$PWD/cloudGame/include
#INCLUDEPATH += $$PWD/cloudGame/include/curl-vc140-static-32_64.7.53.0/lib/native/include
#INCLUDEPATH += $$PWD/cloudGame/include/openssl-vc140-static-32_64.1.1.0/lib/native/include
DEPENDPATH += $$PWD/cloudGame/lib/x64
#DEPENDPATH += $$PWD/cloudGame/include/openssl-vc140-static-32_64.1.1.0/lib/native/libs/x64/static/Release
#DEPENDPATH += $$PWD/cloudGame/include/curl-vc140-static-32_64.7.53.0/lib/native/libs/x64/static/release

#LIBS += -L$$PWD/cloudGame/include/curl-vc140-static-32_64.7.53.0/lib/native/libs/x64/static/release -llibcurl
#LIBS += -L$$PWD/cloudGame/include/openssl-vc140-static-32_64.1.1.0/lib/native/libs/x64/static/Release -llibcrypto
#LIBS += -L$$PWD/cloudGame/include/openssl-vc140-static-32_64.1.1.0/lib/native/libs/x64/static/Release -llibssl
LIBS += -L$$PWD/cloudGame/lib/x64/ -lCLOUDGAME
LIBS += \
      -lUser32 \
      -lDbghelp

DISTFILES += \
    cloudGame/include/lib_json/sconscript

msvc:QMAKE_CXXFLAGS += -execution-charset:utf-8
