#include "mainwindow.h"
#include "glresource.hpp"
#include "glx.hpp"

void TestGLX() {
	GLEffect gle;
	gle.readGLX("test.glx");
	GLint techID = gle.getTechID("TheTech");
	gle.setTechnique(techID, true);
	GLint passID = gle.getPassID("P0");
	gle.setPass(passID);

	SPVDecl decl(new VDecl{
		{0,0, GL_FLOAT, GL_FALSE, 3, (GLuint)VSem::POSITION},
		{0,12, GL_FLOAT, GL_FALSE, 4, (GLuint)VSem::TEXCOORD0}
	});
	gle.setVDecl(decl);
	gle.setUniform(vec4{1,2,3,4}, gle.getUniformID("lowVal"));
	gle.applySetting();

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
