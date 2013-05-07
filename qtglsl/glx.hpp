#pragma once

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <boost/lexical_cast.hpp>
#include "glresource.hpp"
#include "glx_parse.hpp"
#include "dgmath.hpp"

//! OpenGLの値設定関数代理クラス
struct ValueSettingR;
using VSFunc = void (*)(const ValueSettingR&);
using VBFunc = void (*)(GLenum);
struct ValueSettingR {
	ValueSetting::ValueT 	value[4];
	VSFunc					func;

	const static VSFunc cs_func[];

	static void StencilFuncFront(int func, int ref, int mask);
	static void StencilFuncBack(int func, int ref, int mask);
	static void StencilOpFront(int sfail, int dpfail, int dppass);
	static void StencilOpBack(int sfail, int dpfail, int dppass);
	static void StencilMaskFront(int mask);
	static void StencilMaskBack(int mask);

	explicit ValueSettingR(const ValueSetting& s);
	void action() const;
	template <class GF, class... Ts>
	void action(GF gf, Ts...) const {
		// valueのサイズがsizeof...(Ts)と同じ前提
		const auto* ptr = value + sizeof...(Ts);
		gf(boost::get<Ts>(*(--ptr))...);
	}
	bool operator == (const ValueSettingR& s) const;
};
//! OpenGLのBool値設定クラス
struct BoolSettingR {
	GLenum		flag;
	VBFunc		func;

	const static VBFunc cs_func[2];

	explicit BoolSettingR(const BoolSetting& s);
	void action() const;
	bool operator == (const BoolSettingR& s) const;
};

struct VData {
	const static int MAX_STREAM = 4;
	using SPBuffA = SPBuffer[MAX_STREAM];
	using AttrA = GLint[static_cast<int>(VSem::NUM_SEMANTIC)];

	const SPBuffA&	spBuff;
	const AttrA&	attrID;

	VData(const SPBuffA& b, const AttrA& at): spBuff(b), attrID(at) {}
};
//! 頂点宣言
class VDecl {
	public:
		struct VDInfo {
			GLuint	streamID,		//!< 便宜上の)ストリームID
					offset,			//!< バイトオフセット
					elemFlag,		//!< OpenGLの要素フラグ
					bNormalize,		//!< OpenGLが正規化するか(bool)
					elemSize,		//!< 要素数
					semID;			//!< 頂点セマンティクスID
		};
	private:
		using Func = std::function<void (GLuint, const VData::AttrA&)>;
		using FuncL = std::vector<Func>;
		FuncL	_func;						//!< ストリーム毎のサイズを1次元配列で格納 = 0番から並べる
		int		_nEnt[VData::MAX_STREAM+1];	//!< 各ストリームの先頭インデックス

	public:
		VDecl();
		//! 入力: {streamID, offset, GLFlag, bNoramalize, semantics}
		VDecl(std::initializer_list<VDInfo> il);
		//! OpenGLへ頂点位置を設定
		void apply(const VData& vdata) const;
};
using SPVDecl = std::shared_ptr<VDecl>;

using UniVal = boost::variant<bool, int, float, vec3, vec4, Mat23, SPTexture>;
using UniMapStr = std::unordered_map<std::string, UniVal>;
using UniMapID = std::unordered_map<GLint, UniVal>;
using UniEntryMap = std::unordered_set<std::string>;

// OpenGLのレンダリング設定
using Setting = boost::variant<BoolSettingR, ValueSettingR>;
using SettingList = std::vector<Setting>;
//! Tech | Pass の分だけ作成
class TPStructR {
	SPProg			_prog;
	// --- 関連情報(ゼロから構築する場合の設定項目) ---
	//! Attribute: 頂点セマンティクスに対する頂点ID
	/*! 無効なセマンティクスは負数 */
	GLint			_vAttrID[static_cast<int>(VSem::NUM_SEMANTIC)];
	//! Setting: Uniformデフォルト値(texture, vector, float, bool)設定を含む。GLDeviceの設定クラスリスト
	SettingList		_setting;

	UniEntryMap		_noDefValue;	//!< Uniform非デフォルト値エントリリスト (主にユーザーの入力チェック用)
	UniMapID		_defValue;		//!< Uniformデフォルト値と対応するID

	public:
		TPStructR();
		TPStructR(TPStructR&& tp);
		TPStructR(const GLXStruct& gs, int tech, int pass);

		bool findSetting(const Setting& s) const;
		void swap(TPStructR& tp) noexcept;

		const UniMapID& getUniformDefault() const;
		const UniEntryMap& getUniformEntries() const;

		const SPProg& getProgram() const;
		//! OpenGLに設定を適用
		void applySetting() const;
		//! 頂点ポインタを設定 (GLXから呼ぶ)
		void setVertex(const SPVDecl& vdecl, const SPBuffer (&stream)[VData::MAX_STREAM]) const;
		//! 設定差分を求める
		static TPStructR calcDiff(const TPStructR& from, const TPStructR& to);
};
//! 引数の型チェックと同時に出力
struct ArgChecker : boost::static_visitor<> {
	enum TARGET {
		BOOLEAN,
		SCALAR,
		VECTOR,
		NONE
	};
	const static int N_TARGET = 4;
	TARGET _target[N_TARGET];
	const ArgItem* _arg[N_TARGET];
	const std::string& _shName;
	std::ostream& _ost;
	int _cursor = 0;

	ArgChecker(std::ostream& ost, const std::string& shName, const std::vector<ArgItem>& args);
	static TARGET Detect(int type);
	void _checkAndSet(TARGET tgt);
	void operator()(const std::vector<float>& v);
	void operator()(float v);
	void operator()(bool b);
	void finalizeCheck();
};

namespace Bit {
	template <class T0, class T1>
	inline bool ChClear(T0& t, const T1& tbit) {
		bool bRet = t & tbit;
		t &= ~tbit;
		return bRet;
	}
}

//! GLXエフェクト管理クラス
class GLEffect {
	using UseArray = std::vector<std::string>;
	//! アクティブなBoolSetting, ValueSettingを適用
	void _applyShaderSetting() const;

	using TechMap = std::unordered_map<GL16ID, TPStructR>;
	using DiffCache = std::unordered_map<GLDiffID, int>;
	using TechName = std::vector<std::vector<std::string>>;

	TechMap			_techMap;		//!< ゼロから設定を構築する場合の情報や頂点セマンティクス
	DiffCache		_diffCache;		//!< セッティング差分を格納
	TechName		_techName;		//!< Tech名とPass名のセット

	// --------------- 現在アクティブな設定 ---------------
	SPVDecl			_spVDecl;
	SPBuffer		_vBuffer[VData::MAX_STREAM],
					_iBuffer;
	using TPID = boost::optional<int>;
	TPID			_idTech, _idTechCur,
					_idPass, _idPassCur;
	bool			_bDefaultParam;	//!< Tech切替時、trueならデフォルト値読み込み
	UniMapID		_uniMapID,		//!< 設定待ちのエントリ
					_uniMapIDTmp;	//!< 設定し終わったエントリ
	using TPRef = boost::optional<const TPStructR&>;
	TPRef			_tps;

	enum REFLAG {
		REFL_PROGRAM = 0x01,		//!< シェーダーのUse宣言
		REFL_UNIFORM = 0x02,		//!< キャッシュしたUniform値の設定
		REFL_VSTREAM = 0x04,		//!< VStreamの設定
		REFL_ISTREAM = 0x08,		//!< IStreamの設定
		REFL_ALL = 0x0f
	};
	uint32_t		_rflg = REFL_ALL;

	// ----- リフレッシュ関数 -----
	void _refreshProgram();
	void _refreshUniform();
	void _refreshVStream();
	void _refreshIStream();

	public:
		//! GLEffectで発生する例外基底
		struct EC_Base : std::runtime_error {
			using std::runtime_error::runtime_error;
		};
		//! 該当するTechが無い
		struct EC_TechNotFound : EC_Base { using EC_Base::EC_Base; };
		//! 範囲外のPass番号を指定
		struct EC_PassOutOfRange : EC_Base { using EC_Base::EC_Base; };
		//! 頂点Attributeにデータがセットされてない
		struct EC_EmptyAttribute : EC_Base { using EC_Base::EC_Base; };
		//! Uniformにデータがセットされてない
		struct EC_EmptyUniform : EC_Base { using EC_Base::EC_Base; };
		//! Macroにデータがセットされてない
		struct EC_EmptyMacro : EC_Base { using EC_Base::EC_Base; };
		//! GLXファイルの文法エラー
		struct EC_GLXGrammar : EC_Base { using EC_Base::EC_Base; };
		//! 該当するGLXファイルが見つからない
		struct EC_FileNotFound : EC_Base {
			EC_FileNotFound(const std::string& fPath);
		};

		//! システムセマンティクス(2D)
		//! システムセマンティクス(3D)
		//! システムセマンティクス(Both)

		//! Effectファイル(gfx)を読み込む
		void readGLX(const std::string& fPath);
		//! Uniform変数設定 (Tech/Passで指定された名前とセマンティクスのすり合わせを行う)
		GLint getUniformID(const std::string& name);
		template <class T>
		void setUniform(const T& v, GLint id) {
			_rflg |= REFL_UNIFORM;
			_uniMapID.insert(std::make_pair(id, v));
		}

		//! 現在セットされているUniform変数の保存用
		const UniMapID& getUniformMap() const;
		//! セーブしておいたUniform変数群を復元
		void inputParams(const UniMapStr& u);
		void inputParams(const UniMapID& u);
		//! 実際にOpenGLへ各種設定を適用
		void applySetting();

		//! 頂点宣言
		/*! \param[in] decl 頂点定義クラスのポインタ(定数を前提) */
		void setVDecl(const SPVDecl& decl);
		void setVStream(const SPBuffer& sp, int n);
		void setIStream(const SPBuffer& sp);
		//! Tech指定
		/*!	Techを切り替えるとUniformがデフォルトにリセットされる
			切替時に引数としてUniMapを渡すとそれで初期化 */
		void setTechnique(int techID, bool bDefault);
		int getTechID(const std::string& tech) const;
		int getCurTechID() const;
		//! Pass指定
		void setPass(int passID);
		int getPassID(const std::string& pass) const;
		int getCurPassID() const;
};
namespace std {
	template <>
	inline void swap(TPStructR& t0, TPStructR& t1) noexcept {
		t0.swap(t1);
	}
}
