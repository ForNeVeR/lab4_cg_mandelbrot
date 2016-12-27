QT += widgets concurrent gui

HEADERS       = mandelbrotwidget.h \
                renderthread.h \
    mainwindow.h \
    exportdialog.h
SOURCES       = main.cpp \
                mandelbrotwidget.cpp \
                renderthread.cpp \
    mainwindow.cpp \
    exportdialog.cpp

unix:!mac:!vxworks:!integrity:!haiku:LIBS += -lm

QMAKE_CXXFLAGS_RELEASE *= -Ofast

FORMS += \
    mainwindow.ui \
    exportdialog.ui

