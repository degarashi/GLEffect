#pragma once
#include "mainwindow.h"
#include "gldefine.hpp"

class TestGL : public OpenGLWindow {
	Q_OBJECT
	SPEffect	_gle;
	SPVBuffer	_vbo;
	SPIBuffer	_ibo;
	SPTexture	_tex;

	public:
		TestGL();
		void initialize() override;
		void render() override;
};
