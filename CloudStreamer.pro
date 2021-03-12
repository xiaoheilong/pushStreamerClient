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


////////推流版本号控制
#应用图标
#RC_ICONS += icon.ico
#版本号
VERSION = 1.0.0.23
#中文
RC_LANG = 0x0004
# 公司名
QMAKE_TARGET_COMPANY =广州玖的
# 产品名称
QMAKE_TARGET_PRODUCT = pushStreamer
# 详细描述
QMAKE_TARGET_DESCRIPTION = push streamer and deal some servers message
# 版权
QMAKE_TARGET_COPYRIGHT = jiudi



////////
#加入调试信息
QMAKE_CFLAGS_RELEASE += -g
QMAKE_CXXFLAGS_RELEASE += -g
#禁止优化
QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE -= -O2
#release在最后link时默认有"-s”参数，表示"Omit all symbol information from the output file"，因此要去掉该参
CONFIG += release
win32:CONFIG(debug, debug|release):{
  win32:!msvc{
      QMAKE_LFLAGS_RELEASE = -mthreads
  }else{
      message("msvc version debug msvc")
      QMAKE_CFLAGS_RELEASE = -O2 -MD -Zi
      QMAKE_LFLAGS_RELEASE = /INCREMENTAL:NO /DEBUG
      QMAKE_LFLAGS_RELEASE += /MAP
      QMAKE_CFLAGS_RELEASE += /Zi
      QMAKE_LFLAGS_RELEASE += /debug /opt:ref
      QMAKE_LFLAGS_RELEASE += /MAP /DEBUG /opt:ref /INCREMENTAL:NO
  }
}else{
  win32:!msvc{
      QMAKE_LFLAGS_RELEASE = -mthreads
  }else{
      message("msvc version release msvc")
      QMAKE_LFLAGS_RELEASE = /INCREMENTAL:NO /DEBUG
      QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
      QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO

  }
}


SOURCES += \
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
    consumerkeyboardvalue.cpp \
    websocketconnection.cpp \
    recordgameinfo.cpp

HEADERS += \
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
    consumerkeyboardvalue.h \
    websocketConnection.h \
    recordgameinfo.h

FORMS += \
        cloudstreamer.ui \
   # cloudstreamerKeyBorad.ui \
   # cloudstreamerStreamer.ui

WEBSOCKET_INCLUDE_PATH=E:/local/boost_1_75_0
WEBSOCKET_LIBS_PATH=E:/local/boost_1_75_0/bin/vc14.0/lib/#E:/local/boost_1_75_0/lib64-msvc-14.0

INCLUDEPATH += $$PWD/cloudGame/include
INCLUDEPATH += $$PWD/websocketpp
INCLUDEPATH +=$$WEBSOCKET_INCLUDE_PATH
#INCLUDEPATH += $$PWD/cloudGame/include/curl-vc140-static-32_64.7.53.0/lib/native/include
#INCLUDEPATH += $$PWD/cloudGame/include/openssl-vc140-static-32_64.1.1.0/lib/native/include
DEPENDPATH += $$PWD/cloudGame/lib/x64
DEPENDPATH +=$$WEBSOCKET_LIBS_PATH
#DEPENDPATH += $$PWD/cloudGame/include/openssl-vc140-static-32_64.1.1.0/lib/native/libs/x64/static/Release
#DEPENDPATH += $$PWD/cloudGame/include/curl-vc140-static-32_64.7.53.0/lib/native/libs/x64/static/release

#LIBS += -L$$PWD/cloudGame/include/curl-vc140-static-32_64.7.53.0/lib/native/libs/x64/static/release -llibcurl
#LIBS += -L$$PWD/cloudGame/include/openssl-vc140-static-32_64.1.1.0/lib/native/libs/x64/static/Release -llibcrypto
#LIBS += -L$$PWD/cloudGame/include/openssl-vc140-static-32_64.1.1.0/lib/native/libs/x64/static/Release -llibssl
LIBS += -L$$PWD/cloudGame/lib/x64/ -lCLOUDGAME
LIBS += \
      -lUser32 \
      -lDbghelp

LIBS += $$WEBSOCKET_LIBS_PATH\libboost_random-vc140-mt-x64-1_75.lib
LIBS += $$WEBSOCKET_LIBS_PATH\libboost_date_time-vc140-mt-x64-1_75.lib
LIBS += $$WEBSOCKET_LIBS_PATH\libboost_regex-vc140-mt-x64-1_75.lib
DISTFILES += \
    cloudGame/include/lib_json/sconsc4ript

msvc:QMAKE_CXXFLAGS += -execution-charset:utf-8
