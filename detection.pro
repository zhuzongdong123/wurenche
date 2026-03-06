QT       += core gui concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#能力模块
include(utils/utils.pri)
#主程序
include(main/main.pri)
#第三方对接模块
include(third/third.pri)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

msvc{
    QMAKE_CFLAGS += /utf-8
    QMAKE_CXXFLAGS += /utf-8
}

QMAKE_CXXFLAGS_WARN_ON += -wd4828#屏蔽4828警告


#临时文件存放位置
MOC_DIR         = ./temp/moc
UI_DIR          = ./temp/UI
OBJECTS_DIR     = ./temp/obj

#exe图标
DISTFILES += \
    $$PWD/resource/other/app.rc \
    $$PWD/resource/other/tubiao.ico \

RC_FILE += $$PWD/resource/other/app.rc

# 资源文件
RESOURCES += $$PWD/resource/other/myicon.qrc \

#申请大内存
DEFINES += QT_DEPRECATED_WARNINGS
QMAKE_LFLAGS_WINDOWS += /LARGEADDRESSAWARE

#greaterThan(QT_MAJOR_VERSION,4)
#{
#    contains(QT_ARCH,i386)
#    {
#        QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
#        QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO
#        LIBS += -lDbgHelp
#    }
#}

HEADERS += \
    third/mytable.h

SOURCES += \
    third/mytable.cpp
