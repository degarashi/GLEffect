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
	SPVDecl decl(new VDecl{
		{0,0, GL_FLOAT, GL_FALSE, 3, (GLuint)VSem::POSITION},
		{0,12, GL_FLOAT, GL_FALSE, 4, (GLuint)VSem::TEXCOORD0}
	});
	gle.setVDecl(decl);
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
