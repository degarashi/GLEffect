#include "mainwindow.h"
#include "glhead.hpp"
#include <QOpenGLPaintDevice>
#include <QCoreApplication>
#include <QPainter>

OpenGLWindow::OpenGLWindow(QWindow* parent): QWindow(parent), _bUpdatePending(false), _bAnimating(false) {
	setSurfaceType(QWindow::OpenGLSurface);
}
void OpenGLWindow::render(QPainter* painter) {
	Q_UNUSED(painter)
}
void OpenGLWindow::initialize() {}

void OpenGLWindow::render() {
	if(!_device)
		_device.reset(new QOpenGLPaintDevice);

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
	_device->setSize(size());

	QPainter painter(_device.get());
	render(&painter);
}

void OpenGLWindow::renderLater() {
	if(!_bUpdatePending) {
		_bUpdatePending = true;
		QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
	}
}

bool OpenGLWindow::event(QEvent* event) {
	switch(event->type()) {
		case QEvent::UpdateRequest:
			renderNow();
			return true;
		default:
			return QWindow::event(event);
	}
}

void OpenGLWindow::exposeEvent(QExposeEvent* event) {
	Q_UNUSED(event);
	if(isExposed())
		renderNow();
}
void OpenGLWindow::resizeEvent(QResizeEvent* event) {
	Q_UNUSED(event)
	if(isExposed())
		renderNow();
}

void OpenGLWindow::renderNow() {
	if(!isExposed())
		return;

	bool needInit = false;
	_bUpdatePending = false;
	if(!_context) {
		_context.reset(new QOpenGLContext(this));
		_context->setFormat(requestedFormat());
		_context->create();
		needInit = true;
	}
	_context->makeCurrent(this);
	if(needInit) {
		// OpenGL関数群を読み込む
		LoadXGLFunc();
		initialize();
	}

	render();
	_context->swapBuffers(this);

	if(_bAnimating)
		renderLater();
}

void OpenGLWindow::setAnimating(bool bAnim) {
	_bAnimating = bAnim;
	if(bAnim)
		renderLater();
}
