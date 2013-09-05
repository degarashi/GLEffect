#pragma once
#include <QApplication>

#include "mainwindow.h"
#include "gldefine.hpp"
#include "glresource.hpp"
#include "glx.hpp"
#include "spinner/matstack.hpp"
#include "spinner/pose.hpp"
#include "boomstick/rigid2D.hpp"
#include "updator.hpp"
#include "font.hpp"
#include "gpu.hpp"

extern QString BASE_PATH;

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
class Arrow : public IUpdate {
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
		void update(float dt) override;
};

//! 四角ポリゴン表示
class Actor : public IUpdate, public spn::CheckAlign<16,Actor> {
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
		void update(float dt) override;
		boom::geo2d::HRig getHRig() const;
};
//! 文字描画
class TextDraw : public IUpdate {
	HLText		_hlText;
	spn::Vec2	_pos;
	enum Pivot {
		Left = 0x01,
		Right = 0x02,
		HCenter = 0x04,
		Top = 0x08,
		Bottom = 0x10,
		VCenter = 0x20
	};
	uint32_t _pivot;

	public:
		TextDraw();
		void resetText(HText hT);
		void setPos(const spn::Vec2& p);
		void setPivot(uint32_t flag);
		void update(float dt) override;
};

#define mgr_test TestGL::_ref()
class TestGL : public OpenGLWindow, public spn::Singleton<TestGL> {
	Q_OBJECT

	HLFx	_hlFx;
	MStack	_mstack;
	struct DrawAsset {
		GLEffect	*gle;
		MStack		&mstack;
	};
	FontGen _fontGen;

	boom::geo2d::HLRig		_hlFloor[3];
	constexpr static uint32_t invalid = ~0;
	uint32_t				_rmID[3] = {invalid};

	UpdGroup				_updG,
							_drawG;
	constexpr static float	_dt = 0.01f;
	int						_nBox=3, _nIter=5;
	boom::geo2d::HLMdl			_hlMdl;
	boom::geo2d::ConvexModel*	_pMdl;

	bool		_bDelayVSync = false;
	int			_vsyncNum;
	void _applyVSync();
	void _setVSync(int n);

	CCoreID		_coreID;
	using SPTDraw = std::shared_ptr<TextDraw>;
	SPTDraw		_tdraw;
	GPUTime		_gpuTime;
	GPUInfo		_gpuInfo;

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
		static SPVDecl s_vDecl2D;
		TestGL();
		~TestGL();
		void initialize() override;
		void render() override;
		spn::Vec2 qtposToWorld(const spn::Vec2& pos);
		DrawAsset getDrawAsset();
		void addDraw(Priority prio, const SPUpdate& d);
		void addUpd(Priority prio, const SPUpdate& d);

	public slots:
		void resetScene();
		void changeEnv(const SimEnv& e);
		void changeCoeff(const boom::RCoeff& c);
		void changeInitial(const SimInitial& in);
		void changeView(const SimView& v);

		boom::geo2d::HRig getBox(const spn::Vec2& pos);
};
