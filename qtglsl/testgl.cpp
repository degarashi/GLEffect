#include "mainwindow.h"
#include "glresource.hpp"
#include "glx.hpp"

void TestGLX() {
	GLEffect gle;
	gle.readGLX("test.glx");
// 	gle.setTechnique("TestTech");
// 	gle.setPass("TestPass");
// 	gle.applySetting();
// 	gle.setPass("AnotherPass");
// 	gle.applySetting();
// 	gle.setVDecl({0, 0, 4, VSem.POSITION});
// 	gle.setUniform(vec4(1,2,3,4), "the_param");
// 	gle.setMacro("the_entry", 128);

// 	glDrawElements(...);
// 	gle.saveParams();
// 	gle.restoreParams();
}

TestGL::TestGL(QWidget* parent) {

}

void TestGL::initializeGL() {
	// OpenGL関数群を読み込む
	LoadXGLFunc();
	TestGLX();
}
void TestGL::resizeGL(int, int) {}
void TestGL::paintGL() {}
