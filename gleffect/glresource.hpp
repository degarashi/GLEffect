#pragma once
#define BOOST_PP_VARIADICS 1
#include "dgassert.hpp"
#include "glhead.hpp"
#include "gldefine.hpp"
#include <cstdint>
#include <stdexcept>
#include <string>
#include <memory>
#include <vector>
#include <sstream>
#include <boost/format.hpp>
#include <QImage>
#include "spinner/vector.hpp"
#include <boost/variant.hpp>
#include "spinner/resmgr.hpp"

//! Tech:Pass の組み合わせを表す
struct GL16ID {
	union {
		uint16_t	value;
		uint8_t		id[2];
	};
	GL16ID() {}
	GL16ID(int id0, int id1): id{static_cast<uint8_t>(id0),static_cast<uint8_t>(id1)} {}
	bool operator < (const GL16ID& t) const { return value < t.value; }
	bool operator == (const GL16ID& t) const { return value == t.value; }
	operator uint_fast16_t() const { return value; }
};
//! ある(Tech:Pass)から別の(Tech:Pass)への遷移を表す
struct GLDiffID {
	union {
		uint32_t		value;
		struct {
			uint16_t	fromID, toID;
		};
	};
	GLDiffID() {}
	GLDiffID(uint_fast16_t id0, uint_fast16_t id1): fromID(id0), toID(id1) {}
	bool operator < (const GLDiffID& t) const { return value < t.value; }
	bool operator == (const GLDiffID& t) const { return value == t.value; }
};
namespace std {
	template <> struct hash<GL16ID> {
		size_t operator() (const GL16ID& id) const {
			return id.value;
		}
	};
	template <> struct hash<GLDiffID> {
		size_t operator() (const GLDiffID& id) const {
			return id.value;
		}
	};
}

//! OpenGL関連のリソース
/*! Android用にデバイスロスト対応 */
struct IGLResource {
	virtual void onDeviceLost() {}
	virtual void onDeviceReset() {}
	virtual ~IGLResource() {}
};
// ------------------ GL例外クラス ------------------
//! OpenGLに関する全般的なエラー
struct GLE_Error : std::runtime_error {
	using runtime_error::runtime_error;
};
using GLGetIV = decltype(glGetShaderiv);
using GLInfoFunc = decltype(glGetShaderInfoLog);
//! GLSLコンパイル関連のエラー基底
struct GLE_ShProgBase : GLE_Error {
	GLE_ShProgBase(GLGetIV ivF, GLInfoFunc infoF, const std::string& aux, GLuint id);
};
//! GLSLシェーダーコンパイルエラー
struct GLE_ShaderError : GLE_ShProgBase {
	GLE_ShaderError(GLuint id);
};
//! GLSLプログラムリンクエラー
struct GLE_ProgramError : GLE_ShProgBase {
	GLE_ProgramError(GLuint id);
};
//! GLSL変数が見つからないエラー
struct GLE_ParamNotFound : GLE_Error {
	GLE_ParamNotFound(const std::string& name);
};
//! GLSLユーザー変数の型エラー
struct GLE_InvalidArgument : GLE_Error {
	GLE_InvalidArgument(const std::string& shname, const std::string& argname);
};
//! GLXファイルの論理的な記述ミス
struct GLE_LogicalError : GLE_Error {
	using GLE_Error::GLE_Error;
};

// ------------------ GLリソース管理 ------------------
//! GLシェーダークラス
class GLShader : public IGLResource {
	GLuint	_idSh,
			_flag;
	const std::string _source;

	void _initShader();

	public:
		//! 空シェーダーの初期化
		GLShader();
		GLShader(GLuint flag, const std::string& src);
		~GLShader() override;

		bool isEmpty() const;
		int getShaderID() const;
		void onDeviceLost() override;
		void onDeviceReset() override;
};

//! OpenGLバッファクラス
class GLBuffer : public IGLResource {
	GLuint		_buffType,			//!< VERTEX_BUFFERなど
				_drawType,			//!< STATIC_DRAWなどのフラグ
				_stride,			//!< 頂点1つのバイトサイズ
				_idBuff;			//!< OpenGLバッファID
	ByteBuff	_buff;				//!< 再構築の際に必要となるデータ実体

	const static GLuint cs_cnv[];

	void _initBuffer();

	public:
		GLBuffer(GLuint flag, GLuint dtype);
		~GLBuffer() override;
		void onDeviceLost() override;
		void onDeviceReset() override;

		// 全域を書き換え
		void initData(const void* src, size_t nElem, GLuint stride=0);
		void initData(ByteBuff&& buff, GLuint stride=0);
		// 部分的に書き換え
		void updateData(const void* src, size_t nElem, GLuint offset);

		GLuint getBuffID() const;
		GLuint getBuffType() const;
		GLuint getStride() const;
		static GLuint GetUnitSize(GLuint flag);

		void use() const;
};
//! 頂点バッファ
class GLVBuffer : public GLBuffer {
	public:
		GLVBuffer(GLuint dtype);
};

//! インデックスバッファ
class GLIBuffer : public GLBuffer {
	public:
		GLIBuffer(GLuint dtype);
		void initData(const GLubyte* src, size_t nElem);
		void initData(const GLushort* src, size_t nElem);
		void initData(ByteBuff&& buff);
		void initData(const U16Buff& buff);

		void updateData(const GLushort* src, size_t nElem, GLuint offset);
		void updateData(const GLubyte* src, size_t nElem, GLuint offset);
};

//! OpenGLエラーIDとその詳細メッセージ
struct ErrID {
	GLenum				_id;
	const std::string	_msg;
	ErrID(int id, const char* msg);
};

//! OpenGLラッパークラス
class GLDevice : public IGLResource {
	const static ErrID cs_err[5];
	public:
		static struct _Policy_Critical : DGAssert::_Policy_Critical {
			void onOutput(const std::string& msg) const override {
				throw GLE_Error(msg);
			}
		} Policy_Critical;

		using CStr = const char*;
		//! OpenGLのエラーに対して例外又は警告を出す
		template <class... Ts>
		static bool checkError(DGAssert::IPolicy* policy, CStr file, CStr func, int line, const std::string& info, const Ts&... ts) {
			GLenum err;
			bool ret = false;
			while((err = glGetError()) != GL_NO_ERROR) {
				// OpenGLエラー詳細を取得
				std::string cause;
				for(const auto& e : cs_err) {
					if(e._id == err) {
						cause = (boost::format("OpenGL CheckError(%1%)") % e._msg).str();
						break;
					}
				}
				if(cause.empty())
					cause.assign("OpenGL CheckError(unknown errorID)");
				ret |= DGAssert::checkAssert(policy, cause.c_str(), file, func, line, info, ts...);
				if(ret) {
					resetError();
					break;
				}
			}
			return ret;
		}
		//! エラー値を出力しなくなるまでループする
		static void resetError() {
			while(glGetError() != GL_NO_ERROR);
		}
};
#ifdef DEBUG
	// OpenGLに関するアサート集
	#define GL_ACheckArg(...) { static bool bIgnored = false; \
		if(!bIgnored) { bIgnored = GLDevice::checkError(&GLDevice::Policy_Critical, __FILE__, FUNCTIONNAME, __LINE__, __VA_ARGS__); } }
	#define GL_ACheck() GL_ACheckArg("")
	#define GL_AWarnArg(...) GLDevice::checkError(&DGAssert::Policy_Warning, __FILE__, FUNCTIONNAME, __LINE__, __VA_ARGS__));
	#define GL_AWarn() GL_AWarnArg("")
	#define GL_AResetError() GLDevice::resetError();
#else
	#define GL_ACheckArg(...)
	#define GL_ACheck()
	#define GL_AWarnArg(...)
	#define GL_AWarn()
	#define GL_AResetError()
#endif
// Debug/Releaseに関係なくエラーチェックをしたい時用
#define GL_CheckArg(...) GLDevice::checkError(&GLDevice::Policy_Critical, __FILE__, FUNCTIONNAME, __LINE__, __VA_ARGS__);
#define GL_Check() GL_CheckArg("")
#define GL_WarnArg(...) GLDevice::checkError(&DGAssert::Policy_Warning, __FILE__, FUNCTIONNAME, __LINE__, __VA_ARGS__);
#define GL_Warn() GL_WarnArg("")
#define GL_ResetError() GLDevice::resetError();

#define mgr_gl GLRes::_ref()
//! OpenGL関連のリソースマネージャ
class GLRes : public spn::ResMgrN<UPResource, GLRes> {
	using base_type = spn::ResMgrN<UPResource, GLRes>;
	public:
		//! ベースクラスのacquireメソッドを隠す為のダミー
		void acquire();
		//! ファイルからテクスチャを読み込む
		AnotherLHandle<UPTexture> loadTexture(const QString& path, bool bCube=false);
		//! 文字列からシェーダーを作成
		AnotherLHandle<UPShader> makeShader(GLuint flag, const std::string& src);

		using HSh = AnotherSHandle<UPShader>;
		using HLProg = AnotherLHandle<UPProg>;
		//! 複数のシェーダーからプログラムを作成 (vertex, geometry, pixel)
		HLProg makeProgram(HSh vsh, HSh gsh, HSh psh);
		//! 複数のシェーダーからプログラムを作成 (vertex, pixel)
		HLProg makeProgram(HSh vsh, HSh psh);

		//! ファイルからエフェクトの読み込み
		AnotherLHandle<UPEffect> loadEffect(const QString& path);
		//! 頂点バッファの確保
		AnotherLHandle<UPVBuffer> makeVBuffer(GLuint dtype);
		//! インデックスバッファの確保
		AnotherLHandle<UPIBuffer> makeIBuffer(GLuint dtype);

		LHdl _common(const QString& key, std::function<UPResource()> cb);
};

DEF_HANDLE(GLRes, Tex, UPTexture)
DEF_HANDLE(GLRes, Vb, UPVBuffer)
DEF_HANDLE(GLRes, Ib, UPIBuffer)
DEF_HANDLE(GLRes, Buff, UPBuffer)
DEF_HANDLE(GLRes, Prog, UPProg)
DEF_HANDLE(GLRes, Sh, UPShader)
DEF_HANDLE(GLRes, Fx, UPEffect)
DEF_HANDLE(GLRes, Res, UPResource)

//! GLSLプログラムクラス
class GLProgram : public IGLResource {
	HLSh		_shader[ShType::NUM_SHTYPE];
	GLuint		_idProg;

	void _initProgram();

	public:
		GLProgram(HSh vsh, HSh psh) {
			_shader[ShType::VERTEX] = vsh;
			_shader[ShType::PIXEL] = psh;
			_initProgram();
		}
		GLProgram(HSh vsh, HSh gsh, HSh psh) {
			_shader[ShType::VERTEX] = vsh;
			_shader[ShType::PIXEL] = psh;
			_shader[ShType::GEOMETRY] = gsh;
			_initProgram();
		}

		GLProgram();
		~GLProgram() override;
		void onDeviceLost() override;
		void onDeviceReset() override;
		const HLSh& getShader(ShType type) const;
		int getUniformID(const std::string& name) const;
		int getUniformIDNc(const std::string& name) const;
		int getAttribID(const std::string& name) const;
		int getAttribIDNc(const std::string& name) const;
		GLuint getProgramID() const;
		void use() const;
};
//! OpenGLテクスチャインタフェース
/*!	フィルターはNEARESTとLINEARしか無いからboolで管理 */
class IGLTexture : public IGLResource {
	public:
		enum State {
			NotDecided = -1,
			NoMipmap,
			MipmapNear,
			MipmapLinear
		};
	protected:
		GLuint	_idTex;
		int		_iLinearMag,	//!< Linearの場合は1, Nearestは0
				_iLinearMin,
				_iWrapS,
				_iWrapT;
		int		_width,
				_height;
		bool	_bChanged;	//!< 何かフィルタ設定が変更された時にtrue

		const static GLuint cs_Filter[3][2];

		State	_state;
		GLuint	_texFlag;	//!< TEXTURE_2D or TEXTURE_CUBE_MAP
		float	_coeff;

		bool _onDeviceReset();
		IGLTexture(bool bCube);
		void _applyFilter();

	public:
		~IGLTexture();
		int getWidth() const;
		int getHeight() const;
		GLint getTextureID() const;
		void setFilter(bool bLinearMag, bool bLinearMin);
		void setAnisotropicCoeff(float coeff);
		void setUVWrap(GLuint s, GLuint t);
		void onDeviceLost() override;
		void use();				//!< 現在のテクスチャユニットにBind
		void use(int n);		//!< テクスチャユニット番号を指定してBind
		void use_end() const;
		void applyFilter();
		void setMipmap(State level);

		bool isMipmap() const;
		bool isCubemap() const;
};

template <class T, int N>
struct Pack {
	T	val[N];
	Pack() = default;
	Pack(std::initializer_list<T> il) {
		T* pVal = val;
		for(auto& a : il)
			*pVal++ = a;
	}
	Pack(const T& t) {
		for(auto& a : val)
			a = t;
	}

	bool operator == (const Pack& p) const {
		for(int i=0 ; i<6 ; i++) {
			if(val[i] != p.val[i])
				return false;
		}
		return true;
	}
};

//! ファイルから生成したテクスチャ
/*! DeviceLost時:
	一旦バッファにコピーして後で復元 */
class TexFile : public IGLTexture {
	using QS6 = Pack<QString, 6>;
	boost::variant<QString, QS6>	_fPath;

	public:
		//! Cube時: 連番ファイル名から作成
		TexFile(const QString& path, bool bCube);
		TexFile(const QString& path0, const QString& path1, const QString& path2,
									const QString& path3, const QString& path4, const QString& path5);
		void onDeviceReset() override;
		bool operator == (const TexFile& t) const;
};

//! ユーザー定義のユニークテクスチャ
/*! DeviceLost時:
	再度ファイルから読み出す */
class TexUser : public IGLTexture {
	using QI6 = Pack<QImage, 6>;
	boost::variant<QImage, QI6>		_image;

	public:
		TexUser(const QImage& img);
		TexUser(const QImage& img0, const QImage& img1, const QImage& img2,
				const QImage& img3, const QImage& img4, const QImage& img5);
		void onDeviceReset() override;
		bool operator == (const TexUser& t) const;
};

//! デバッグ用テクスチャ模様生成インタフェース
class ITDGen {
	protected:
		using UPByte = std::unique_ptr<GLubyte>;
		UPByte	_buff;
		int		_width, _height;

		ITDGen(int w, int h);

	public:
		const GLubyte* getPtr() const;
		int getWidth() const;
		int getHeight() const;
};
//! 2色チェッカー
class TDChecker : public ITDGen {
	public:
		TDChecker(const spn::Vec4& col0, const spn::Vec4& col1, int nDivW, int nDivH, int w, int h);
};
//! カラーチェッカー
/*! 準モンテカルロで色を決定 */
class TDCChecker : public ITDGen {
	public:
		TDCChecker(int nDivW, int nDivH, int w, int h);
};
//! ベタ地と1テクセル枠
class TDBorder : public ITDGen {
	public:
		TDBorder(const spn::Vec4& col, const spn::Vec4& bcol, int w, int h);
};

//! デバッグ用のチェッカーテクスチャ
/*! DeviceLost時:
	再度生成し直す */
class TexDebug : public IGLTexture {
	// デバッグ用なので他との共有を考えず、UniquePtrとする
	std::unique_ptr<ITDGen>	_upGen;
	using ITD6 = Pack<ITDGen*, 6>;
	boost::variant<ITDGen*, ITD6>	_gen;

	public:
		TexDebug(ITDGen* gen, bool bCube);
		void onDeviceReset() override;
		bool operator == (const TexDebug& t) const;
};
