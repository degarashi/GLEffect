#pragma once
#include <cstdint>
#include <stdexcept>
#include <string>
#include <memory>
#include "glhead.hpp"

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

//! OpenGLエラーIDとその詳細メッセージ
struct ErrID {
	GLenum				_id;
	const std::string	_msg;
	ErrID(int id, const char* msg);
};

//! OpenGLラッパークラス
class GLDevice : public IGLResource {
	const static ErrID cs_err[];
	public:
		static void checkError(const std::string& what);
};

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
