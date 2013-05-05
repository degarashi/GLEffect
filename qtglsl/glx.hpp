#pragma once

#include <functional>
#include <unordered_map>
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
struct VDecl {
	class VDArray {
		using FUNC = std::function<void (const VData&)>;
		FUNC _func;

		public:
			VDArray() {}
			VDArray(const VDArray& v) = default;
			VDArray& operator = (const VDArray& v) {
				_func = v._func;
				return *this;
			}
			/*! \param[in] streamID		(便宜上の)ストリームID
				\param[in] elemFlag		OpenGLの要素フラグ
				\param[in] bNormalize	OpenGLが正規化するか(bool)
				\param[in] offset		バイトオフセット
				\param[in] semID		頂点セマンティクスID */
			VDArray(GLuint streamID, GLuint elemFlag, GLuint bNormalize, GLuint offset, GLuint semID);
			void apply(const VData& vdata) const {
				_func(vdata);
			}
	};
	using VDArrayL = std::vector<VDArray>;
	VDArrayL	_ar;

	VDecl();
	// {streamID, offset, GLFlag, bNoramalize, semantics}
	VDecl(std::initializer_list<VDArray> il);
	//! OpenGLへ頂点位置を設定
	void apply(const VData& vdata) const;
};

using DefVal = std::pair<std::string, boost::variant<GLTexture, vec4, float, bool>>;
using Setting = boost::variant<DefVal, BoolSettingR, ValueSettingR>;
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

	public:
		TPStructR();
		TPStructR(TPStructR&& tp);
		TPStructR(const GLXStruct& gs, int tech, int pass);

		bool findSetting(const Setting& s) const;
		void swap(TPStructR& tp) noexcept;

		//! OpenGLに設定を適用
		void applySetting() const;
		//! 頂点ポインタを設定 (GLXから呼ぶ)
		void setVertex(const VDecl& vdecl, const SPBuffer (&stream)[VData::MAX_STREAM]) const;
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
//! GLXエフェクト管理クラス
class GLEffect {
	using UseArray = std::vector<std::string>;
	//! アクティブなBoolSetting, ValueSettingを適用
	void _applyShaderSetting() const;

	using TechMap = std::unordered_map<GL16ID, TPStructR>;
	using DiffCache = std::unordered_map<GLDiffID, int>;

	TechMap		_techMap;		//!< ゼロから設定を構築する場合の情報や頂点セマンティクス
	DiffCache	_diffCache;		//!< セッティング差分を格納
	VDecl		_vDecl;			//!< 現在アクティブな頂点定義
	SPBuffer	_vBuffer[VData::MAX_STREAM];

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
		void setUniform(const vec4& v, const std::string& name);
		void setUniform(const vec3& v, const std::string& name);
		void setUniform(const Mat23& m, const std::string& name);
		//! 現在セットされているUniform変数の保存
		void* saveParams() const;
		//! セーブしておいたUniform変数群を復元
		void restoreParams(void* ptr);
		//! 実際にOpenGLへ各種設定を適用
		void applySetting();

		//! 頂点宣言
		void setVDecl(const VDecl* pDecl);
		//! Tech指定
		void setTechnique(const std::string& tech);
		//! Pass指定
		void setPass(int n);
		void setPass(const std::string& );
		//! マクロ変数指定 (float, vector, string)
		template <class T>
		void setMacro(const std::string& entry, const T& value) {
			boost::lexical_cast<std::string>(value);
		}
};
namespace std {
	template <>
	inline void swap(TPStructR& t0, TPStructR& t1) noexcept {
		t0.swap(t1);
	}
}
