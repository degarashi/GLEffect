#pragma once
#include <cstdint>
#include <stdexcept>
#include <string>
#include <memory>
#include <vector>
#include <sstream>
#include <boost/format.hpp>
#include "glhead.hpp"
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
using SPShader = std::shared_ptr<GLShader>;

using ByteBuff = std::vector<uint8_t>;
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
		//! データの初期化無し
		GLBuffer(GLuint flag, GLuint dtype, GLuint stride);
		//! データをbuffで初期化
		GLBuffer(GLuint flag, GLuint dtype, GLuint stride, ByteBuff&& buff);
		GLBuffer(GLuint flag, GLuint dtype, GLuint stride, const void* src, size_t length);

		~GLBuffer() override;
		void onDeviceLost() override;
		void onDeviceReset() override;

		void setBufferData(const void* src, size_t length);
		void setBufferData(ByteBuff&& buff);

		GLuint getBuffID() const;
		GLuint getBuffType() const;
		GLuint getStride() const;
		static GLuint GetUnitSize(GLuint flag);

		void use() const;
};
using SPBuffer = std::shared_ptr<GLBuffer>;

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
		template <class FMT>
		static FMT& Tmp(FMT& fmt) { return fmt; }
		template <class FMT, class T, class... Ts>
		static FMT& Tmp(FMT& fmt, const T& t, const Ts&... ts) {
			fmt % t;
			return Tmp(fmt, ts...);
		}

		static void output_Throw(const std::string& msg) {
			throw GLE_Error(msg);
		}
		static void output_Print(const std::string& msg) {
			std::cout << msg;
		}

		//! OpenGLのエラーに対して例外又は警告を出す
		template <class OUTF, class... Ts>
		static void checkError(OUTF outf, const char* file, const char* func, int line, const std::string& what, const Ts&... ts) {
			GLenum err;
			while((err = glGetError()) != GL_NO_ERROR) {
				for(const auto& e : cs_err) {
					if(e._id == err) {
						using std::endl;
						std::stringstream ss;
						ss << "GLDevice check failed! (" << e._msg << ") at" << endl
							<< "file=" << file << endl
							<< "func=" << func << endl
							<< "line=" << line << endl;
						boost::format fmt(what);
						ss << Tmp(fmt, ts...).str();

						outf(ss.str());
					}
				}
				outf("unknown OpenGL error!");
			}
		}
		//! エラー値を出力しなくなるまでループする
		static void resetError() {
			while(glGetError() != GL_NO_ERROR);
		}
};
#ifdef DEBUG
	// OpenGLに関するアサート集
	#define GL_ACheckArg(...) GLDevice::checkError(GLDevice::output_Throw, __FILE__, __func__, __LINE__, __VA_ARGS__);
	#define GL_ACheck() GL_ACheckArg("")
	#define GL_AWarnArg(...) GLDevice::checkError(GLDevice::output_Print, __FILE__, __func__, __LINE__, __VA_ARGS__);
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
#define GL_CheckArg(...) GLDevice::checkError(GLDevice::output_Throw, __FILE__, __func__, __LINE__, __VA_ARGS__);
#define GL_Check() GL_CheckArg("")
#define GL_WarnArg(...) GLDevice::checkError(GLDevice::output_Print, __FILE__, __func__, __LINE__, __VA_ARGS__);
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

	void _setShader(int n) {}
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
using SPProg = std::shared_ptr<GLProgram>;

//! OpenGLテクスチャクラス
class GLTexture : public IGLResource {
	GLuint _idTex;

	public:
		GLuint getTextureID() const;
		bool operator == (const GLTexture& t) const;
};
using SPTexture = std::shared_ptr<GLTexture>;
