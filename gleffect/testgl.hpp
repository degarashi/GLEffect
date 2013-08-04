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
//! 点ジョイント
class Joint : public boom::geo2d::IResist {
	spn::Vec2 _lcFrom = spn::Vec2(0),	//! ジョイント元座標(ローカル)
				_wTo = spn::Vec2(0);	//! ジョイント先(グローバル)
	bool	_bEnable = false;
	public:
		void setLcFrom(const spn::Vec2& from);
		void setTo(const spn::Vec2& to);
		void setEnable(bool b);
		void resist(boom::geo2d::RForce::F& acc, const boom::geo2d::Rigid& r, int index, const boom::geo2d::CResult& cr) const override;
};
//! 矢印ポリゴン
class Arrow : public IDraw {
	HLVb	_hlVb;
	HLIb	_hlIb;
	HLTex	_hlTex;
	float	_width = 0.02f;
	spn::Vec2	_vFrom = spn::Vec2(0),
				_vTo = spn::Vec2(0);

	public:
		Arrow();
		void setFrom(const spn::Vec2& v);
		void setTo(const spn::Vec2& v);
		void draw(GLEffect* glf, MStack& ms) override;
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

	spn::noseq_list<SPUpdate, uint32_t>	_updL;
	spn::noseq_list<SPDraw, uint32_t>	_drawL;
	constexpr static float	_dt = 0.01f;
	int						_nBox=3, _nIter=5;
	boom::geo2d::HLMdl			_hlMdl;
	boom::geo2d::ConvexModel*	_pMdl;

	using SPGrav = std::shared_ptr<boom::geo2d::resist::Gravity>;
	using SPAir = std::shared_ptr<boom::geo2d::resist::Air>;
	using SPArrow = std::shared_ptr<Arrow>;
	using SPJoint = std::shared_ptr<Joint>;
	SPGrav					_spGrav;
	SPAir					_spAir;
	SPArrow					_spArrow;
	uint32_t				_arrowID;
	SPJoint					_spJoint;
	//! grab中のboxハンドル
	boom::geo2d::HRig		_hRig;
	//! grabしている物体のローカル位置
	spn::Vec2				_lcPos;

	//! 前回フレームのビューサイズ
	float		_widthH = 320,
				_heightH = 240;
	//! 前回フレームの射影行列
	spn::Mat44	_invP;

	void _release();

	protected:
		void mousePressEvent(QMouseEvent* e) override;
		void mouseMoveEvent(QMouseEvent* e) override;
		void mouseReleaseEvent(QMouseEvent* e) override;
	public:
		TestGL();
		~TestGL();
		void initialize() override;
		void render() override;
		spn::Vec2 qtposToWorld(const spn::Vec2& pos);

	public slots:
		void resetScene();
		void changeEnv(const SimEnv& e);
		void changeCoeff(const boom::RCoeff& c);
		void changeInitial(const SimInitial& in);

		boom::geo2d::HRig getBox(const spn::Vec2& pos);
		spn::noseq_list<SPDraw, uint32_t>& refDrawList();
};
