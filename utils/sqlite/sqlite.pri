#主要提供数据库连接能力
QT       +=sql
INCLUDEPATH += \
    $$PWD \

HEADERS += \
    $$PWD/dbthread.h \
    $$PWD/mysqlite.h

SOURCES += \
    $$PWD/dbthread.cpp \
    $$PWD/mysqlite.cpp

