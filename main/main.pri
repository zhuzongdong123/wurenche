#共同的组件(类)
include(common/common.pri)

#业务部分：包含登录，界面显示，数据交互等
INCLUDEPATH += \
    $$PWD \
    $$PWD/common \

FORMS += \
    $$PWD/cameraparamssetting.ui \
    $$PWD/formeventmark.ui \
    $$PWD/formpilenumupdate.ui \
    $$PWD/formsetting.ui \
    $$PWD/mainwindow.ui \
    $$PWD/widgetmappage.ui \
    $$PWD/widgetstatusbar.ui

HEADERS += \
    $$PWD/cameraparamssetting.h \
    $$PWD/formeventmark.h \
    $$PWD/formpilenumupdate.h \
    $$PWD/formsetting.h \
    $$PWD/mainwindow.h \
    $$PWD/paramsdao.h \
    $$PWD/widgetmappage.h \
    $$PWD/widgetstatusbar.h

SOURCES += \
    $$PWD/cameraparamssetting.cpp \
    $$PWD/formeventmark.cpp \
    $$PWD/formpilenumupdate.cpp \
    $$PWD/formsetting.cpp \
    $$PWD/main.cpp \
    $$PWD/mainwindow.cpp \
    $$PWD/paramsdao.cpp \
    $$PWD/widgetmappage.cpp \
    $$PWD/widgetstatusbar.cpp
