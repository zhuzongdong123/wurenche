#主要提供加载web页面能力

QT       += webengine webchannel webenginewidgets

INCLUDEPATH += $$PWD
HEADERS += \
    $$PWD/document.h \
    $$PWD/webview.h

SOURCES += \
    $$PWD/webview.cpp
