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

Actor::Actor(HMdl hMdl) {
	_init();
	_hlRig = mgr_rigidgl.acquireRigid(Rigid::New(hMdl));

	// 剛体ハンドル作成
	auto spImp = IResist::sptr(new resist::Impact());
	_hlRig.ref()->addR(spImp);
	_rmID = mgr_rigidgl.addA(_hlRig.get());
}

Actor::Actor(HMdl hMdl, const spn::Pose2D& ps): Actor(hMdl) {
	RPose& rp = _hlRig.ref()->refPose();
	static_cast<spn::Pose2D&>(rp) = ps;
	rp.setAccel(Vec2(0,0));
	rp.setRotAccel(0);
	rp.setVelocity(Vec2(0,0));
	rp.setRotVel(0);
	_hlRig.ref()->refPose().setOfs(ps.getOffset());
}

void Actor::update(float /*dt*/) {}
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
HRig Actor::getHRig() const { return _hlRig.get(); }

TestGL::TestGL(): _spGrav(new resist::Gravity), _spAir(new resist::Air) {}
TestGL::~TestGL() {
	_release();
}
void TestGL::_release() {
	if(_rmID[0] != invalid) {
		for(auto id : _rmID)
			mgr_rigidgl.remB(id);
		_rmID[0] = invalid;
	}
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

	resetScene();
}
void TestGL::render() {
	glClearColor(0,0,0.5f, 1.0f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	int w = width(),
		h = height();
	glViewport(0,0,w,h);

	_mstack.clear();
	_mstack.push(spn::Mat44::PerspectiveFovLH(spn::DEGtoRAD(90), float(w)/h, 0.01f, 100.0f));

	for(int i=0 ; i<_nIter ; i++)
		mgr_rigidgl.simulate(_dt/_nIter);
	for(auto& sp : _updL)
		sp->update(_dt);
	auto* pFx = _hlFx.ref().get();
	for(auto& sp : _drawL)
		sp->draw(pFx, _mstack);
}
void TestGL::resetScene() {
	_release();
	_updL.clear();
	_drawL.clear();

	using spn::Vec2;
	using spn::Pose2D;
	if(!_hlMdl.valid()) {
		// モデルハンドル作成
		constexpr float sx = 0.5f,
						sy = 0.3f;
		ConvexModel* c0 = ConvexModel::New({Vec2(0,0), Vec2(0,sy), Vec2(sx,sy), Vec2(sx,0)});
		// 重心を中心点に据える
		Vec2 ofs0 = c0->getCenter();
		c0->addOffset(-ofs0);
		_hlMdl = mgr_rigidgl.acquireModel(c0);
		_pMdl = c0;
	}

	// 箱を用意
	for(int i=0 ; i<_nBox ; i++) {
		SPActor sp(Actor::New(_hlMdl.get(), Pose2D{Vec2(i*0.0f,i*0.31f), spn::DEGtoRAD(0.f), Vec2(1,1)}));
		_updL.push_back(sp);
		_drawL.push_back(sp);
		auto& tmp = sp->getHRig().ref();
		tmp->addR(_spGrav);
		tmp->addR(_spAir);
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
void TestGL::changeEnv(const SimEnv& e) {
	_spGrav->setGravity(e.gravity);
	_spAir->setAir(e.air.x, e.air.y);
}
void TestGL::changeCoeff(const boom::RCoeff& c) {
	mgr_rigidgl.setCoeff(c);
}
void TestGL::changeInitial(const SimInitial& in) {
	_nBox = in.nbox;
	_nIter = in.nIter;
	if(_hlMdl.valid()) {
		auto cnv = static_cast<boom::Cache<ConvexCore>*>(_pMdl);
		cnv->setCacheRatio(in.mass, TagArea());
		cnv->setCacheRatio(in.inertia, TagInertia());
	}
}
void TestGL::mousePressEvent(QMouseEvent* e) {
	emit mousePressEv(e);
}
