#include "mainwindow.h"
#include "glresource.hpp"
#include "glx.hpp"

namespace {
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

		// 頂点フォーマット定義
		SPVDecl decl(new VDecl{
			{0,0, GL_FLOAT, GL_FALSE, 3, (GLuint)VSem::POSITION},
			{0,12, GL_FLOAT, GL_FALSE, 4, (GLuint)VSem::TEXCOORD0}
		});
		gle.setVDecl(decl);
		GL_ACheck()
		gle.setUniform(vec4{1,2,3,4}, gle.getUniformID("lowVal"));

		// 頂点定義
		struct TmpV {
			vec3 pos;
			vec4 tex;
		};
		TmpV tmpV[] = {
			{
				{-1,-1,-1},
				{0,0,0,0}
			},
			{
				{-1,1,-1},
				{0,1,0,0}
			},
			{
				{1,1,-1},
				{1,1,0,0}
			},
			{
				{1,-1,-1},
				{1,0,0,0}
			}
		};
		SPVBuffer spV(new GLVBuffer(GL_STATIC_DRAW));
		spV->initData(tmpV, countof(tmpV), sizeof(TmpV));
		gle.setVStream(spV, 0);

		// インデックス定義
		GLubyte tmpI[] = {
			0,1,2,
			2,3,0
		};
		SPIBuffer spI(new GLIBuffer(GL_STATIC_DRAW));
		spI->initData(tmpI, countof(tmpI));
		gle.setIStream(spI);

		gle.drawIndexed(GL_TRIANGLES, 6, 0);

	// 	gle.saveParams();
	// 	gle.restoreParams();
	}
}

TestGL::TestGL() {}
void TestGL::initialize() {
	std::cout	<< "OpenGL Version: " << ::glGetString(GL_VERSION) << std::endl
				<< "OpenGL Vendor: " << ::glGetString(GL_VENDOR) << std::endl
				<< "OpenGL Renderer: " << ::glGetString(GL_RENDERER) << std::endl
				<< "GLSL Version: " << ::glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl
				<< "Extensions: " << ::glGetString(GL_EXTENSIONS) << std::endl;
	TestGLX();
}
void TestGL::render() {
	::glClearColor(0,0,1.0f, 1.0f);
	::glClear(GL_COLOR_BUFFER_BIT);
}
