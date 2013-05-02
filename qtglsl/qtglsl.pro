#-------------------------------------------------
#
# Project created by QtCreator 2013-05-02T13:42:23
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qtglsl
TEMPLATE = app


SOURCES += main.cpp\
		mainwindow.cpp \
	testgl.cpp \
	glshader.cpp \
	glx.cpp \
    gltexture.cpp \
    glx_output.cpp \
    glx_parse.cpp

HEADERS  += mainwindow.h \
	glext.h \
	glresource.hpp \
	glhead.hpp \
	glx.hpp \
	dgmath.hpp \
	glx_parse.hpp \
	glx_macro.hpp

FORMS    += mainwindow.ui

QMAKE_CXXFLAGS += -std=c++11 -Wno-sequence-point -Wno-unused-parameter -Wno-unused-variable
QMAKE_CXX = g++-4.80
QMAKE_LINK = g++-4.80
LIBS += -lGLU
