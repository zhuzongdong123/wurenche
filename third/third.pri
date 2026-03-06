#第三方设备的对接：海康相机，RTK模块，平整仪等
LIBS += -L$$PWD/depend/MVS/Development/Libraries/win64/ -lMvCameraControl
INCLUDEPATH += $$PWD/depend/MVS/Development/Includes

INCLUDEPATH += \
    $$PWD \

HEADERS += \
    $$PWD/accelerometer.h \
    $$PWD/flatnesstester.h \
    $$PWD/hikcamera.h \
    $$PWD/imagesavethread.h \
    $$PWD/memoryimage.h

SOURCES += \
    $$PWD/accelerometer.cpp \
    $$PWD/flatnesstester.cpp \
    $$PWD/hikcamera.cpp \
    $$PWD/imagesavethread.cpp \
    $$PWD/memoryimage.cpp

