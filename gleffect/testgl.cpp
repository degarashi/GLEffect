#include <QMessageBox>
#include <QApplication>
#include <QDir>
#include "testgl.hpp"

Actor::~Actor() {
	mgr_rigidgl.remA(_rmID);
}

void Actor::_init() {
	using spn::Vec3;
	using spn::Vec4;
	// 頂点定義
	struct TmpV {
		Vec3 pos;
		Vec4 tex;
	};
	constexpr float sx = 0.5f/2,
					sy = 0.3f/2;
	TmpV tmpV[] = {
		{
			Vec3{-sx,-sy,0},
			Vec4{0,1,0,0}
		},
		{
			Vec3{-sx,sy,0},
			Vec4{0,0,0,0}
		},
		{
			Vec3{sx,sy,0},
			Vec4{1,0,0,0}
		},
		{
			Vec3{sx,-sy,0},
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
using namespace boom::geo2d;
void Center::resist(RForce::F &acc, const Rigid &r, int /*index*/, const CResult& /*cr*/) const {
	auto& p = r.getPose();
	acc.linear += -p.getOffset() * 0.9f;
}

Actor::Actor() {
	_init();

	// モデルハンドル作成
	constexpr float sx = 0.5f,
					sy = 0.3f;
	ConvexModel* c0 = ConvexModel::New({Vec2(0,0), Vec2(0,sy), Vec2(sx,sy), Vec2(sx,0)});
//	((boom::Cache<ConvexCore>*)c0)->setCacheRatio(3.5f, TagArea());
//	((boom::Cache<ConvexCore>*)c0)->setCacheRatio(3.5f, TagInertia());
	// 重心を中心点に据える
	Vec2 ofs0 = c0->getCenter();
	c0->addOffset(-ofs0);
	HLMdl hlC0 = mgr_rigidgl.acquireModel(c0);
	_hlRig = mgr_rigidgl.acquireRigid(Rigid::New(hlC0));

	// 剛体ハンドル作成
	auto spGrav = IResist::sptr(new resist::Gravity(Vec2(0,-7.f)));
	_hlRig.ref()->addR(spGrav);
	auto spImp = IResist::sptr(new resist::Impact());
	_hlRig.ref()->addR(spImp);
	auto spAir = IResist::sptr(new resist::Air(0.01f, 0.01f));
	_hlRig.ref()->addR(spAir);
//	_hlRig.ref()->addR(IResist::sptr(new Center()));

	_rmID = mgr_rigidgl.addA(_hlRig.get());
}

Actor::Actor(const spn::Pose2D& ps): Actor() {
	RPose& rp = _hlRig.ref()->refPose();
	static_cast<spn::Pose2D&>(rp) = ps;
	_hlRig.ref()->refPose().setOfs(ps.getOffset());
//	_hlRig.ref()->refPose().setVelocity(Vec2(2,2));
//	_hlRig.ref()->refPose().setRotVel(10.f);
}

void Actor::update(float /*dt*/) {
	auto& m = _hlRig.ref()->refPose().getToWorld();
	auto m2 = m;
}
void Actor::draw(GLEffect* glf, MStack& ms) {
	auto& m = _hlRig.ref()->refPose().getToWorld();
	ms.push(m.convert44());

	GLint id = glf->getUniformID("mTrans");
	glf->setUniform(ms.top(), id);
	id = glf->getUniformID("tDiffuse");
	glf->setUniform(_hlTex, id);

	glf->setVStream(_hlVb.get(), 0);
	glf->setIStream(_hlIb.get());
	glf->drawIndexed(GL_TRIANGLES, 6, 0);
	GL_ACheck()

	ms.pop();
}

TestGL::TestGL() {}
TestGL::~TestGL() {
//	mgr_rigidgl.remA(_rmID);
}

void TestGL::initialize() {
	_hlFx = mgr_gl.loadEffect("test.glx");
	std::cout	<< "OpenGL Version: " << ::glGetString(GL_VERSION) << std::endl
				<< "OpenGL Vendor: " << ::glGetString(GL_VENDOR) << std::endl
				<< "OpenGL Renderer: " << ::glGetString(GL_RENDERER) << std::endl
				<< "GLSL Version: " << ::glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl
				<< "Extensions: " << ::glGetString(GL_EXTENSIONS) << std::endl
				<< "--------------------------------------------------------" << std::endl;
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
	UPVDecl decl(new VDecl{
		{0,0, GL_FLOAT, GL_FALSE, 3, (GLuint)VSem::POSITION},
		{0,12, GL_FLOAT, GL_FALSE, 4, (GLuint)VSem::TEXCOORD0}
	});
	pFx->setVDecl(std::move(decl));
	GL_ACheck()
	pFx->setUniform(spn::Vec4{1,2,3,4}, pFx->getUniformID("lowVal"));

	using spn::Vec2;
	using spn::Pose2D;

	for(int i=0 ; i<8 ; i++) {
		SPActor sp(Actor::New(Pose2D{Vec2(i*0.0f,i*0.31f), spn::DEGtoRAD(0.f), Vec2(1,1)}));
		_updL.push_back(sp);
		_drawL.push_back(sp);
	}
	PointL pl[3] = {
		{Vec2(-2,-0.9f), Vec2(2,-0.9f), Vec2(2,-2), Vec2(-2,-2)},
		{Vec2(-2,1), Vec2(-1,1), Vec2(-1,-1), Vec2(-2,-1)},
		{Vec2(1,1), Vec2(2,1), Vec2(2,-1), Vec2(1,-1)}
	};
	for(int i=0 ; i<3 ; i++) {
		// 床を用意
		ConvexModel* c0 = ConvexModel::New(pl[i]);
		auto cen = c0->getCenter();
		c0->addOffset(-cen);
		HLMdl hlC0 = mgr_rigidgl.acquireModel(c0);
		_hlFloor[i] = mgr_rigidgl.acquireRigid(Rigid::New(hlC0));
		_rmID[i] = mgr_rigidgl.addB(_hlFloor[i].get());
		_hlFloor[i].ref()->setOfs(cen);
	}
}
void TestGL::render() {
	glClearColor(0,0,1.0f, 1.0f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	int w = width(),
		h = height();
	glViewport(0,0,w,h);

	_mstack.clear();
	_mstack.push(spn::Mat44::PerspectiveFovLH(spn::DEGtoRAD(90), float(w)/h, 0.01f, 100.0f));

	const float dt = 0.01f;
	const int iter = 5;
	for(int i=0 ; i<iter ; i++)
		mgr_rigidgl.simulate(dt/iter);
	for(auto& sp : _updL)
		sp->update(dt);
	auto* pFx = _hlFx.ref().get();
	for(auto& sp : _drawL)
		sp->draw(pFx, _mstack);
}
