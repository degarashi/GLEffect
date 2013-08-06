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
	glformat.cpp \
	font_common.cpp \
	font_qt_dep.cpp \
	fontgen.cpp \
    gpu.cpp

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
	common.hpp \
	font_qt_dep.hpp \
	font_base.hpp \
	font.hpp \
	lane.hpp \
	updator.hpp \
    gpu.hpp

FORMS    += mainwindow.ui

CONFIG += c++11
QMAKE_CXXFLAGS_DEBUG += -DDEBUG -ggdb3 -Wno-sequence-point -Wno-unused-parameter -Wno-unused-variable
QMAKE_CXX = 'clang++'
QMAKE_LINK = 'clang++'
LIBS += -lGLU /tmp/boomstick_build/libboomstick.a /tmp/spinner_build/libspinner.a /usr/local/lib/libboost_system.a /usr/local/lib/libboost_regex.a
QMAKE_LIBDIR += /tmp/spinner_build/ \
				/tmp/boomstick_build/
QMAKE_INCDIR += ./boomstick/ \
				./spinner/
