#include "mainwindow.h"
#include "glresource.hpp"
#include "glx.hpp"

void TestGLX() {
	GLEffect gle("test.glx");
	GLint techID = gle.getTechID("TheTech");
	GL_ACheck()
	gle.setTechnique(techID, true);
	GL_ACheck()
	GLint passID = gle.getPassID("P0");
	GL_ACheck()
	gle.setPass(passID);
	GL_ACheck()

	SPVDecl decl(new VDecl{
		{0,0, GL_FLOAT, GL_FALSE, 3, (GLuint)VSem::POSITION},
		{0,12, GL_FLOAT, GL_FALSE, 4, (GLuint)VSem::TEXCOORD0}
	});
	gle.setVDecl(decl);
	GL_ACheck()
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
