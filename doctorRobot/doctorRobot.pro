QT += network gui core widgets serialport

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QMAKE_CXXFLAGS += -std=c++11 -DWITH_OPENCV -DWITH_OPENCV2

INCLUDEPATH +=  /home/loo/work/build/imx8m/mynteye/include \
                /home/loo/work/build/imx8m/opencv/include  \
                /home/loo/work/build/imx8m/libjpeg-turbo-2.0.0/include

LIBS += -L/home/loo/work/build/imx8m/mynteye/lib -leSPDI -lmynteyed   \
        -L/home/loo/work/build/imx8m/opencv/lib -lopencv_core \
        -L/home/loo/work/build/imx8m/libjpeg-turbo-2.0.0/lib64 -ljpeg -lturbojpeg

HEADERS += \
    controller.h \
    server.h \
    arm.h \
    ui/homewidget.h \
    ui/infowidget.h \
    ui/mainwindow.h \
    ui/settingwidget.h \
    ui/sidewidget.h \
    ui/titlewidget.h \
    gpio.h \
    actuator.h \
    innfos_net.h \
    car.h

SOURCES += \
    controller.cpp \
    main.cpp \
    server.cpp \
    arm.cpp \
    ui/homewidget.cpp \
    ui/infowidget.cpp \
    ui/mainwindow.cpp \
    ui/settingwidget.cpp \
    ui/sidewidget.cpp \
    ui/titlewidget.cpp \
    gpio.cpp \
    actuator.cpp \
    car.cpp

RESOURCES += \
    ui/resc.qrc

DISTFILES += \
    version \
    robot.conf
