#pragma once
#include <QApplication>

#include "mainwindow.h"
#include "gldefine.hpp"
#include "glresource.hpp"
#include "glx.hpp"

class TestGL : public OpenGLWindow {
	Q_OBJECT
	HLFx	_hlFx;
	HLVb	_hlVb;
	HLIb	_hlIb;
	HLTex	_hlTex;

	public:
		TestGL();
		void initialize() override;
		void render() override;
};
