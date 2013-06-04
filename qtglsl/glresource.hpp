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

#define countof(elem) static_cast<int>(sizeof((elem))/sizeof((elem)[0]))

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
using GLGetIV = void (*)(GLuint, GLenum, GLint*);
using GLInfoFunc = void (*)(GLuint, GLsizei, GLsizei*, GLchar*);
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

enum ShType : unsigned int {
	VERTEX, GEOMETRY, PIXEL,
	NUM_SHTYPE
};
//! シェーダーIDに対するOpenGL定数
const static GLuint c_glShFlag[ShType::NUM_SHTYPE] = {
	GL_VERTEX_SHADER, GL_GEOMETRY_SHADER, GL_FRAGMENT_SHADER
};

//! GLSLプログラムクラス
class GLProgram : public IGLResource {
	SPShader	_shader[ShType::NUM_SHTYPE];
	GLuint		_idProg;

	void _setShader(int) {}
	template <class... Ts>
	void _setShader(int n, const SPShader& sp0, const Ts&... sp) {
		_shader[n] = sp0;
		_setShader(n+1, sp...);
	}
	void _initProgram();

	public:
		template <class... Ts>
		GLProgram(const Ts&... sp) {
			static_assert(sizeof...(Ts) <= (size_t)ShType::NUM_SHTYPE, "invalid arguments");
			_setShader(0, sp...);
			_initProgram();
		}
		~GLProgram() override;
		void onDeviceLost() override;
		void onDeviceReset() override;
		const SPShader& getShader(ShType type) const;
		int getUniformID(const std::string& name) const;
		int getUniformIDNc(const std::string& name) const;
		int getAttribID(const std::string& name) const;
		int getAttribIDNc(const std::string& name) const;
		GLuint getProgramID() const;
		void use() const;
};
//! OpenGLテクスチャインタフェース
class IGLTexture : IGLResource {
	protected:
		GLuint	_idTex,
				_filMin,
				_filMag;
		int		_width,
				_height;

		bool _onDeviceReset();
		void _applyFilter();
		IGLTexture(GLuint filMin, GLuint filMag);

	public:
		~IGLTexture();
		int getWidth() const;
		int getHeight() const;
		GLint getTextureID() const;
		GLuint getFilterMin() const;
		GLuint getFilterMag() const;
		void setFilter(GLuint fMin, GLuint fMag);
		void onDeviceLost() override;
		void use() const;		//!< 現在のテクスチャユニットにBind
		void use(int n) const;	//!< テクスチャユニット番号を指定してBind
		static void use_end();
};
//! ファイルから生成したテクスチャ
/*! DeviceLost時:
	一旦バッファにコピーして後で復元 */
class TexFile : public IGLTexture {
	QString	_fPath;

	public:
		TexFile(const QString& path, GLuint filMin=GL_LINEAR_MIPMAP_LINEAR, GLuint filMag=GL_LINEAR);
		void onDeviceReset() override;
		bool operator == (const TexFile& t) const;
};
//! ユーザー定義のユニークテクスチャ
/*! DeviceLost時:
	再度ファイルから読み出す */
class TexUser : public IGLTexture {
	QImage	_image;

	public:
		TexUser(const QString& path);
		bool operator == (const TexUser& t) const;
		void onDeviceReset() override;
};
