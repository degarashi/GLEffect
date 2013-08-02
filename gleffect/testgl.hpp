#pragma once
#include <QApplication>

#include "mainwindow.h"
#include "gldefine.hpp"
#include "glresource.hpp"
#include "glx.hpp"
#include "spinner/matstack.hpp"
#include "spinner/pose.hpp"
#include "boomstick/rigid2D.hpp"

extern QString BASE_PATH;
// -------------- 間に合わせ実装。後でちゃんとした物にする --------------
using MStack = spn::MatStack<spn::Mat44, spn::MatStackTag::PushLeft>;
struct IDraw {
	virtual void draw(GLEffect* glf, MStack& ms) = 0;
};
struct IUpdate {
	virtual void update(float dt) = 0;
};

#define mgr_rigidgl reinterpret_cast<RigidMgrGL&>(boom::geo2d::ModelMgr::_ref())
class RigidMgrGL : public boom::geo2d::RigidRes, public boom::geo2d::RigidMgr {
	public:
		using RigidMgr::RigidMgr;
};
class Center : public boom::geo2d::IResist {
	public:
		void resist(boom::geo2d::RForce::F& acc, const boom::geo2d::Rigid& r, int index, const boom::geo2d::CResult& cr) const override;
};

//! 四角ポリゴン表示
class Actor : public IDraw, public IUpdate, public spn::CheckAlign<16,Actor> {
	HLVb		_hlVb;
	HLIb		_hlIb;
	HLTex		_hlTex;

	boom::geo2d::HLRig		_hlRig;		//!< 姿勢を表現するハンドル
	uint32_t				_rmID;		//!< RigidMgrに_hlRigを登録した時のID

	void _init();

	public:
		Actor(boom::geo2d::HMdl hMdl);
		Actor(boom::geo2d::HMdl hMdl, const spn::Pose2D& ps);
		~Actor();
		void draw(GLEffect* glf, MStack& ms) override;
		void update(float dt) override;
		boom::geo2d::HRig getHRig() const;
};
using SPUpdate = std::shared_ptr<IUpdate>;
using SPDraw = std::shared_ptr<IDraw>;
using SPActor = std::shared_ptr<Actor>;

class TestGL : public OpenGLWindow {
	Q_OBJECT
	HLFx	_hlFx;
	MStack	_mstack;
	boom::geo2d::HLRig		_hlFloor[3];
	constexpr static uint32_t invalid = ~0;
	uint32_t				_rmID[3] = {invalid};

	std::vector<SPUpdate>	_updL;
	std::vector<SPDraw>		_drawL;
	constexpr static float	_dt = 0.01f;
	int						_nBox=3, _nIter=5;
	boom::geo2d::HLMdl		_hlMdl;
	boom::geo2d::ConvexModel*	_pMdl;

	using SPGrav = std::shared_ptr<boom::geo2d::resist::Gravity>;
	using SPAir = std::shared_ptr<boom::geo2d::resist::Air>;
	SPGrav					_spGrav;
	SPAir					_spAir;

	void _release();

	protected:
		void mousePressEvent(QMouseEvent* e) override;
	public:
		TestGL();
		~TestGL();
		void initialize() override;
		void render() override;

	signals:
		void mousePressEv(QMouseEvent* e);
	public slots:
		void resetScene();
		void changeEnv(const SimEnv& e);
		void changeCoeff(const boom::RCoeff& c);
		void changeInitial(const SimInitial& in);
};
