#-------------------------------------------------
#
# Project created by QtCreator 2013-05-02T13:42:23
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gleffect
TEMPLATE = app


SOURCES += main.cpp\
		mainwindow.cpp \
	testgl.cpp \
	glshader.cpp \
	glx.cpp \
	gltexture.cpp \
	glx_output.cpp \
	glx_parse.cpp \
	glbuffer.cpp \
	openglwindow.cpp \
	glx_parse2.cpp \
	glassert.cpp \
	glresource.cpp \
    glframebuffer.cpp \
    glformat.cpp

HEADERS  += mainwindow.h \
	glext.h \
	glresource.hpp \
	glhead.hpp \
	glx.hpp \
	glx_parse.hpp \
	glx_macro.hpp \
	testgl.hpp \
	gldefine.hpp \
	dgassert.hpp \
    glformat.hpp \
    glformat_const.hpp \
    common.hpp 

FORMS    += mainwindow.ui

CONFIG += c++11
QMAKE_CXXFLAGS_DEBUG += -DDEBUG -ggdb3 -Wno-sequence-point -Wno-unused-parameter -Wno-unused-variable
QMAKE_CXXFLAGS += -D_WIN32 -msse2
QMAKE_CXX = 'i386-mingw32-g++'
QMAKE_LINK = 'i386-mingw32-g++'
QMAKE_AR_CMD = 'i386-mingw32-ar'
LIBS += -lboomstick -lspinner -lboost_system -lglu32 -lopengl32
QMAKE_LIBDIR += /tmp/spinner_build/ \
				/tmp/boomstick_build/ \
				/home/slice/local/lib/
QMAKE_INCDIR += ./boomstick/ \
				./spinner/ \
				/home/slice/local/include/
