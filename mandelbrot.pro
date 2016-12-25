QT += widgets concurrent

HEADERS       = mandelbrotwidget.h \
                renderthread.h
SOURCES       = main.cpp \
                mandelbrotwidget.cpp \
                renderthread.cpp

unix:!mac:!vxworks:!integrity:!haiku:LIBS += -lm

QMAKE_CXXFLAGS_RELEASE *= -Ofast

