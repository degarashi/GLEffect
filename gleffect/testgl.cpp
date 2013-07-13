#include <QMessageBox>
#include <QApplication>
#include <QDir>

#include "testgl.hpp"
#include "glresource.hpp"
#include "glx.hpp"
#include "spinner/matrix.hpp"

TestGL::TestGL() {}
void TestGL::initialize() {
	_hlFx = mgr_gl.loadEffect("test.glx");
	std::cout	<< "OpenGL Version: " << ::glGetString(GL_VERSION) << std::endl
				<< "OpenGL Vendor: " << ::glGetString(GL_VENDOR) << std::endl
				<< "OpenGL Renderer: " << ::glGetString(GL_RENDERER) << std::endl
				<< "GLSL Version: " << ::glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl
				<< "Extensions: " << ::glGetString(GL_EXTENSIONS) << std::endl;
	auto* pFx = _hlFx.ref().get();
	GLint techID = pFx->getTechID("TheTech");
	GL_ACheck()
	pFx->setTechnique(techID, true);
	GL_ACheck()
	GLint passID = pFx->getPassID("P0");
	GL_ACheck()
	pFx->setPass(passID);
	GL_ACheck()
	// 頂点フォーマット定義
	SPVDecl decl(new VDecl{
		{0,0, GL_FLOAT, GL_FALSE, 3, (GLuint)VSem::POSITION},
		{0,12, GL_FLOAT, GL_FALSE, 4, (GLuint)VSem::TEXCOORD0}
	});
	pFx->setVDecl(decl);
	GL_ACheck()
	pFx->setUniform(spn::Vec4{1,2,3,4}, pFx->getUniformID("lowVal"));

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
	_hlVb = mgr_gl.makeVBuffer(GL_STATIC_DRAW);
	_hlVb.ref()->initData(tmpV, countof(tmpV), sizeof(TmpV));

	// インデックス定義
	GLubyte tmpI[] = {
		0,1,2,
		2,3,0
	};
	_hlIb = mgr_gl.makeIBuffer(GL_STATIC_DRAW);
	_hlIb.ref()->initData(tmpI, countof(tmpI));

	// テクスチャmgr_gl読み込み
	_hlTex = mgr_gl.loadTexture("sample.png", false);
//	_tex.reset(new TexDebug(new TDChecker(spn::Vec4(1,1,1,1), spn::Vec4(0,0,0,0), 24,24,256,256), false));
	_hlTex.ref()->setFilter(false,false);
}
void TestGL::render() {
	glClearColor(0,0,1.0f, 1.0f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	int w = width(),
		h = height();
	glViewport(0,0,w,h);
	if(!_hlVb.valid() || !_hlTex.valid())
		return;

	static float angle = 0;
	spn::Mat44 m4 = spn::Mat44::RotationZ(spn::DEGtoRAD(angle));
	m4 *= spn::Mat44::PerspectiveFovLH(spn::DEGtoRAD(90), float(w)/h, 0.01f, 100.0f);

	auto* pFx = _hlFx.ref().get();
	GLint id = pFx->getUniformID("mTrans");
	pFx->setUniform(m4, id);
	id = pFx->getUniformID("tDiffuse");
	pFx->setUniform(_hlTex, id);
	angle += 1.0f;

	pFx->setVStream(_hlVb.get(), 0);
	pFx->setIStream(_hlIb.get());
	pFx->drawIndexed(GL_TRIANGLES, 6, 0);
	GL_ACheck()
}
