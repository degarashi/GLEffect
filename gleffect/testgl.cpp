#include <QMessageBox>
#include <QApplication>
#include <QDir>
#include <QMouseEvent>
#include "testgl.hpp"
#include "spinner/plane.hpp"
#include <boost/lexical_cast.hpp>

QString BASE_PATH("./data");

using namespace boom::geo2d;
using namespace spn;

// ----------------------- TextDraw -----------------------
TextDraw::TextDraw(): _pos(0), _pivot(Left|Top) {}
void TextDraw::resetText(HText hT) {
	_hlText = hT;
}
void TextDraw::setPos(const Vec2& p) {
	_pos = p;
}
void TextDraw::setPivot(uint32_t flag) {
	_pivot = flag;
}
void TextDraw::update(float /*dt*/) {
	if(_hlText.valid()) {
		auto da = mgr_test.getDrawAsset();
		auto& t = _hlText.ref();
		SizeF s = t.getSize();
		s.width = 2.f / mgr_test.width();
		s.height = 2.f / mgr_test.height();
		Mat44 m = Mat44::Scaling(s.width, s.height, 1, 1);
		m *= Mat44::Translation(Vec3(-1,1,0));
		// オフセットの設定
		GLint techID = da.gle->getTechID("TheFont");
		da.gle->setTechnique(techID, true);
		GLint passID = da.gle->getPassID("P0");
		da.gle->setPass(passID);
		GLint id = da.gle->getUniformID("mTrans");
		da.gle->setUniform(m, id);

		t.draw(da.gle);
	}
}

// ----------------------- Joint -----------------------
void Joint::setLcFrom(const Vec2& from) {
	_lcFrom = from;
}
void Joint::setTo(const Vec2& to) {
	_wTo = to;
}
void Joint::setEnable(bool b) {
	_bEnable = b;
}
void Joint::resist(RForce::F &acc, const Rigid &r, int /*index*/, const CResult& /*cr*/) const {
	constexpr float maxlen = 0.3f;
	if(_bEnable) {
		Vec2 wfrom = r.toWorld(_lcFrom);
		Vec2 v = _wTo - wfrom;
		float len = v.length();
		if(len < 1e-6f)
			return;
		v /= len;
		float spr = 0;
		const boom::geo2d::RPose& rp = r.getPose();
		float dump = -v.dot(rp.getVelocAt(wfrom));
		if(len > maxlen) {
			dump = std::max(0.f, dump);
			dump *= 10;
			spr = std::max(0.f, len-maxlen) * 10;
		} else if(len > maxlen-0.05f) {
			dump *= 1;
		}else {
			dump *= 0;
		}
		RForce::F f = Rigid::CalcForce(r.getPose(), wfrom, v*(spr+dump));
		acc += f;
	}
}

struct TmpV {
	Vec3 pos;
	Vec4 tex;
};
// ----------------------- Arrow -----------------------
Arrow::Arrow() {
	_hlVb = mgr_gl.makeVBuffer(GL_DYNAMIC_DRAW);

	GLubyte tmpI[] = {
		0,1,2,
		2,3,0
	};
	_hlIb = mgr_gl.makeIBuffer(GL_STATIC_DRAW);
	_hlIb.ref()->use()->initData(tmpI, countof(tmpI));

	_hlTex = mgr_gl.loadTexture(QString(BASE_PATH) + "/arrowI.png", false);
	_hlTex.ref()->use()->setFilter(IGLTexture::NoMipmap, false,false);
}
void Arrow::setFrom(const spn::Vec2& v) {
	_vFrom = v;
}
void Arrow::setTo(const spn::Vec2& v) {
	_vTo = v;
}
void Arrow::update(float /*dt*/) {
	auto da = mgr_test.getDrawAsset();
	Vec2 to(_vTo - _vFrom);
	float len = to.length();
	if(len < 1e-5f)
		return;
	Vec2 toI = to * spn::_sseRcp22Bit(len),
		ax = toI * boom::cs_mRot90[1];
	const float f05 = _width * 0.5f,
					z = 0.99f;
	TmpV tmpV[] = {
		{
			(_vFrom+ax*f05).asVec3(z),
			Vec4{0,1,0,0}
		},
		{
			(_vTo+ax*f05).asVec3(z),
			Vec4{0,0,0,0}
		},
		{
			(_vTo+ax*-f05).asVec3(z),
			Vec4{1,0,0,0}
		},
		{
			(_vFrom+ax*-f05).asVec3(z),
			Vec4{1,1,0,0}
		}
	};
	_hlVb.ref()->use()->initData(tmpV, countof(tmpV), sizeof(TmpV));

	GLint techID = da.gle->getTechID("TheTech");
	da.gle->setTechnique(techID, true);
	GLint passID = da.gle->getPassID("P0");
	da.gle->setPass(passID);

	GLint id = da.gle->getUniformID("mTrans");
	da.gle->setUniform(da.mstack.top(), id);
	id = da.gle->getUniformID("tDiffuse");
	da.gle->setUniform(_hlTex, id);

	da.gle->setVDecl(TestGL::s_vDecl2D);
	da.gle->setVStream(_hlVb.get(), 0);
	da.gle->setIStream(_hlIb.get());
	da.gle->drawIndexed(GL_TRIANGLES, 6, 0);
	GL_ACheck()
}
// ----------------------- Actor -----------------------
Actor::~Actor() {
	mgr_rigidgl.remA(_rmID);
}

void Actor::_init() {
	// 頂点定義
	constexpr float sx = 0.5f/2,
					sy = 0.3f/2;
	TmpV tmpV[] = {
		{
			Vec3{-sx,-sy,1},
			Vec4{0,1,0,0}
		},
		{
			Vec3{-sx,sy,1},
			Vec4{0,0,0,0}
		},
		{
			Vec3{sx,sy,1},
			Vec4{1,0,0,0}
		},
		{
			Vec3{sx,-sy,1},
			Vec4{1,1,0,0}
		}
	};
	_hlVb = mgr_gl.makeVBuffer(GL_STATIC_DRAW);
	_hlVb.ref()->use()->initData(tmpV, countof(tmpV), sizeof(TmpV));

	// インデックス定義
	GLubyte tmpI[] = {
		0,1,2,
		2,3,0
	};
	_hlIb = mgr_gl.makeIBuffer(GL_STATIC_DRAW);
	_hlIb.ref()->use()->initData(tmpI, countof(tmpI));

	// テクスチャmgr_gl読み込み
	_hlTex = mgr_gl.loadTexture(QString(BASE_PATH) + "/brick.jpg", false);
//	_tex.reset(new TexDebug(new TDChecker(spn::Vec4(1,1,1,1), spn::Vec4(0,0,0,0), 24,24,256,256), false));
	_hlTex.ref()->use()->setFilter(IGLTexture::NoMipmap, true,true);
}

Actor::Actor(HMdl hMdl) {
	_init();
	_hlRig = mgr_rigidgl.acquireRigid(Rigid::NewUF(hMdl));

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

void Actor::update(float /*dt*/) {
	auto da = mgr_test.getDrawAsset();

	GLint techID = da.gle->getTechID("TheTech");
	GL_ACheck()
	da.gle->setTechnique(techID, true);
	GL_ACheck()
	GLint passID = da.gle->getPassID("P0");
	GL_ACheck()
	da.gle->setPass(passID);
	GL_ACheck()

	auto& m = _hlRig.ref()->refPose().getToWorld();
	da.mstack.push(m.convert44());

	GLint id = da.gle->getUniformID("mTrans");
	da.gle->setUniform(da.mstack.top(), id);
	id = da.gle->getUniformID("tDiffuse");
	da.gle->setUniform(_hlTex, id);

	da.gle->setVDecl(TestGL::s_vDecl2D);
	da.gle->setVStream(_hlVb.get(), 0);
	da.gle->setIStream(_hlIb.get());
	da.gle->drawIndexed(GL_TRIANGLES, 6, 0);
	GL_ACheck()

	da.mstack.pop();
}

HRig Actor::getHRig() const { return _hlRig.get(); }
// ---------------------- TestGL ----------------------
TestGL::TestGL(): _fontGen({512,512}), _spGrav(new resist::Gravity), _spAir(new resist::Air) {}
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
// 頂点フォーマット定義
SPVDecl TestGL::s_vDecl2D(
	new VDecl{
			{0,0, GL_FLOAT, GL_FALSE, 3, (GLuint)VSem::POSITION},
			{0,12, GL_FLOAT, GL_FALSE, 4, (GLuint)VSem::TEXCOORD0}
	}
);
void TestGL::initialize() {
	mgr_gl.onDeviceReset();
	_gpuTime.onDeviceReset();
	_gpuInfo.onDeviceReset();

	_coreID = _fontGen.makeCoreID("Arial", CCoreID(12, 10, 0, false, 0));
	_tdraw = SPTDraw(new TextDraw());

	_hlFx = mgr_gl.loadEffect(QString(BASE_PATH) + "/test.glx");
	_spArrow = std::make_shared<Arrow>();
	_spJoint = std::make_shared<Joint>();

	resetScene();
	SetSwapInterval(1);
}
void TestGL::render() {
	if(!_hlFx)
		return;
	_gpuTime.onFrameBegin();

	glClearColor(0,0,0.5f, 1.0f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	int w = width(),
		h = height();
	glViewport(0,0,w,h);

	_mstack.clear();
	_mstack.push(spn::Mat44::PerspectiveFovLH(spn::DEGtoRAD(90), float(w)/h, 0.01f, 100.0f));
	_invP = _mstack.top();
	_invP.invert();
	_widthH = w * 0.5f;
	_heightH = h * 0.5f;

	if(_hRig) {
		Vec2 wpos = _hRig.ref()->toWorld(_lcPos);
		_spArrow->setFrom(wpos);
	}

	for(int i=0 ; i<_nIter ; i++)
		mgr_rigidgl.simulate(_dt/_nIter);
	struct Visitor {
		float _dt;
		Visitor(float dt): _dt(dt) {}
		bool operator()(const SPUpdate& sp) const {
			return sp->update0(_dt);
		}
	};
	Visitor v(_dt);
	_updG.iterate(v);
	_drawG.iterate(v);
	_gpuTime.onFrameEnd();

	auto &ver = _gpuInfo.version(),
		&slver = _gpuInfo.glslVersion(),
		&dver = _gpuInfo.driverVersion();
	auto tm = _gpuTime.getTime();
	std::string tms = boost::lexical_cast<std::string>(tm);
	std::string str((boost::format("OpenGL Version: %1%.%2%\nOpenGL Vendor: %3%\nOpenGL Renderer: %4%\nGLSL Version: %5%.%6%\nDriver Version: %7%\nDrawTime: %8%ns")
						% ver.major % ver.minor
					 % _gpuInfo.vendor() % _gpuInfo.renderer() % slver.major % slver.minor % dver.major % _gpuTime.getTime()).str());
	HLText hlT = _fontGen.createText(_coreID, str);
	_tdraw->resetText(hlT.get());
}
TestGL::DrawAsset TestGL::getDrawAsset() {
	return DrawAsset{_hlFx.ref().get(), _mstack};
}

void TestGL::resetScene() {
	mgr_gl.onDeviceLost();
	mgr_gl.onDeviceReset();

	_release();
	_updG.clear();
	_drawG.clear();

	addDraw(0x00, SPUpdate(_tdraw));

	using spn::Vec2;
	using spn::Pose2D;
	if(!_hlMdl.valid()) {
		// モデルハンドル作成
		constexpr float sx = 0.5f,
						sy = 0.3f;
		auto c0 = ConvexModel::NewUF({Vec2(0,0), Vec2(0,sy), Vec2(sx,sy), Vec2(sx,0)});
		// 重心を中心点に据える
		Vec2 ofs0 = c0->getCenter();
		c0->addOffset(-ofs0);
		_pMdl = c0.get();
		_hlMdl = mgr_rigidgl.acquireModel(std::move(c0));
	}

	// 箱を用意
	for(int i=0 ; i<_nBox ; i++) {
		auto act(Actor::NewS(_hlMdl.get(), Pose2D{Vec2(i*0.0f,i*0.31f), spn::DEGtoRAD(0.f), Vec2(1,1)}));
		SPUpdate sp(act);
		_drawG.add(0x00, sp);
		auto& tmp = act->getHRig().ref();
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
		auto c0 = ConvexModel::NewUF(pl[i]);
		auto cen = c0->getCenter();
		c0->addOffset(-cen);
		HLMdl hlC0 = mgr_rigidgl.acquireModel(std::move(c0));
		_hlFloor[i] = mgr_rigidgl.acquireRigid(Rigid::NewUF(hlC0));
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
	if(e->button() == Qt::LeftButton) {
		QPoint pt = e->pos();
		Vec2 ptv(pt.x(), pt.y());
		HRig hr = getBox(ptv);
		ptv = qtposToWorld(ptv);
		if(hr.valid()) {
			Rigid& r = *hr.ref();
			_lcPos = r.getPose().toLocal(ptv);
			_spJoint->setLcFrom(_lcPos);
			_spJoint->setTo(r.getPose().getOffset());
			_spJoint->setEnable(true);
			hr.ref()->addR(_spJoint, 0xbeef);
			_spArrow->setAsAlive();
			_drawG.add(0x00, _spArrow);
			_spArrow->setFrom(ptv);
			_spArrow->setTo(ptv);
			_hRig = hr;
		}
	}
}
void TestGL::mouseMoveEvent(QMouseEvent* e) {
	if(_hRig.valid()) {
		Vec2 v(e->pos().x(), e->pos().y());
		v = qtposToWorld(v);
		_spJoint->setTo(v);
		_spArrow->setTo(v);
	}
}
void TestGL::mouseReleaseEvent(QMouseEvent* e) {
	if(e->button() == Qt::LeftButton) {
		if(_hRig) {
			_spJoint->setEnable(false);
			_spArrow->destroy();
			_hRig.ref()->remR(0xbeef);
			_hRig = HRig();
		}
	}
}

using namespace spn;
HRig TestGL::getBox(const Vec2& pos) {
	Vec2 tmp = qtposToWorld(pos);

	for(auto itr=mgr_rigidgl.cbeginA() ; itr!=mgr_rigidgl.cendA() ; ++itr) {
		const Rigid& r = *itr;
		if(r.isInner(tmp))
			return (*itr);
	}
	return HRig();
}
Vec2 TestGL::qtposToWorld(const spn::Vec2& pos) {
	// qt -> screen
	AVec4	tposF((pos.x - _widthH) / _widthH,
					-(pos.y - _heightH) / _heightH,
					0, 1),
			tposB(tposF.x, tposF.y, 1, 1);

	// screen -> world
	tposF *= _invP;
	tposF *= _sseRcp22Bit(tposF.w);
	tposB *= _invP;
	tposB *= _sseRcp22Bit(tposB.w);

	auto plane = APlane::FromPtDir(Vec3(0,0,1), Vec3(0,0,-1));
	float rf = plane.dot((const AVec3&)tposF),
			rb = plane.dot((const AVec3&)tposB);
	float r = std::fabs(rf) / (std::fabs(rf) + std::fabs(rb));
	AVec4 tmp = tposF + (tposB - tposF) * r;
	return Vec2(tmp.x, tmp.y);
}
void TestGL::addDraw(Priority prio, const SPUpdate& d) {
	_drawG.add(prio, d);
}
void TestGL::addUpd(Priority prio, const SPUpdate& d) {
	_updG.add(prio, d);
}
