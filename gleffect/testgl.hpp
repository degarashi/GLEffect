#pragma once
#include <QApplication>

#include "mainwindow.h"
#include "gldefine.hpp"
#include "glresource.hpp"
#include "glx.hpp"
#include "spinner/matstack.hpp"
#include "spinner/pose.hpp"
#include "boomstick/rigid2D.hpp"

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
		Actor();
		Actor(const spn::Pose2D& ps);
		~Actor();
		void draw(GLEffect* glf, MStack& ms) override;
		void update(float dt) override;
};
using SPUpdate = std::shared_ptr<IUpdate>;
using SPDraw = std::shared_ptr<IDraw>;
using SPActor = std::shared_ptr<Actor>;

class TestGL : public OpenGLWindow {
	Q_OBJECT
	HLFx	_hlFx;
	MStack	_mstack;
	boom::geo2d::HLRig		_hlFloor[3];
	uint32_t				_rmID[3];

	std::vector<SPUpdate>	_updL;
	std::vector<SPDraw>		_drawL;

	public:
		TestGL();
		~TestGL();
		void initialize() override;
		void render() override;
};
