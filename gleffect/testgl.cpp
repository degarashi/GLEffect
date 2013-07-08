#include <QMessageBox>
#include <QApplication>
#include <QDir>

#include "testgl.hpp"
#include "glresource.hpp"
#include "glx.hpp"
#include "spinner/matrix.hpp"

TestGL::TestGL() {}
void TestGL::initialize() {
	_gle.reset(new GLEffect("test.glx"));
	std::cout	<< "OpenGL Version: " << ::glGetString(GL_VERSION) << std::endl
				<< "OpenGL Vendor: " << ::glGetString(GL_VENDOR) << std::endl
				<< "OpenGL Renderer: " << ::glGetString(GL_RENDERER) << std::endl
				<< "GLSL Version: " << ::glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl
				<< "Extensions: " << ::glGetString(GL_EXTENSIONS) << std::endl;
	GLint techID = _gle->getTechID("TheTech");
	GL_ACheck()
	_gle->setTechnique(techID, true);
	GL_ACheck()
	GLint passID = _gle->getPassID("P0");
	GL_ACheck()
	_gle->setPass(passID);
	GL_ACheck()
	// 頂点フォーマット定義
	SPVDecl decl(new VDecl{
		{0,0, GL_FLOAT, GL_FALSE, 3, (GLuint)VSem::POSITION},
		{0,12, GL_FLOAT, GL_FALSE, 4, (GLuint)VSem::TEXCOORD0}
	});
	_gle->setVDecl(decl);
	GL_ACheck()
	_gle->setUniform(spn::Vec4{1,2,3,4}, _gle->getUniformID("lowVal"));

	using spn::Vec3;
	using spn::Vec4;
	// 頂点定義
	struct TmpV {
		Vec3 pos;
		Vec4 tex;
	};
	TmpV tmpV[] = {
		{
			Vec3{-1,-1,0},
			Vec4{0,1,0,0}
		},
		{
			Vec3{-1,1,0},
			Vec4{0,0,0,0}
		},
		{
			Vec3{1,1,0},
			Vec4{1,0,0,0}
		},
		{
			Vec3{1,-1,0},
			Vec4{1,1,0,0}
		}
	};
	_vbo.reset(new GLVBuffer(GL_STATIC_DRAW));
	_vbo->initData(tmpV, countof(tmpV), sizeof(TmpV));

	// インデックス定義
	GLubyte tmpI[] = {
		0,1,2,
		2,3,0
	};
	_ibo.reset(new GLIBuffer(GL_STATIC_DRAW));
	_ibo->initData(tmpI, countof(tmpI));

	// テクスチャ読み込み
	_tex.reset(new TexDebug(new TDChecker(spn::Vec4(1,1,1,1), spn::Vec4(0,0,0,0), 24,24,256,256), false));
	_tex->setFilter(false,false);
}
void TestGL::render() {
	glClearColor(0,0,1.0f, 1.0f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	int w = width(),
		h = height();
	glViewport(0,0,w,h);
	if(!_vbo || !_tex)
		return;

	static float angle = 0;
	spn::Mat44 m4 = spn::Mat44::RotationZ(spn::DEGtoRAD(angle));
	m4 *= spn::Mat44::PerspectiveFovLH(spn::DEGtoRAD(90), float(w)/h, 0.01f, 100.0f);
	GLint id = _gle->getUniformID("mTrans");
	_gle->setUniform(m4, id);
	id = _gle->getUniformID("tDiffuse");
	_gle->setUniform(_tex, id);
	angle += 1.0f;

	_gle->setVStream(_vbo, 0);
	_gle->setIStream(_ibo);
	_gle->drawIndexed(GL_TRIANGLES, 6, 0);
	GL_ACheck()
}
