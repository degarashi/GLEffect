#include "glresource.hpp"

// OpenGL関数群の定義
#define GLDEFINE(name,type)		type name;
#include "glfunc.inc"
#undef GLDEFINE

// OpenGL関数ロード
#define GLDEFINE(name,type)		name = (type)glXGetProcAddress((const GLubyte*)#name); \
		if(!name) throw std::runtime_error(std::string("error on loading GL function \"") + #name + '\"');
	void LoadXGLFunc() {
		#include "glfunc.inc"
	}
#undef GLDEFINE

GLE_ShProgBase::GLE_ShProgBase(GLGetIV ivF, GLInfoFunc, const std::string& aux, GLuint id): GLE_Error("") {
	int logSize, length;
	ivF(id, GL_INFO_LOG_LENGTH, &logSize);

	std::unique_ptr<char> pBuff(new char[logSize]);
	glGetShaderInfoLog(id, logSize, &length, pBuff.get());

	(GLE_Error&)*this = GLE_Error(aux + pBuff.get());
}

GLE_ShaderError::GLE_ShaderError(GLuint id): GLE_ShProgBase(glGetShaderiv, glGetShaderInfoLog, "compile shader failed: ", id) {}
GLE_ProgramError::GLE_ProgramError(GLuint id): GLE_ShProgBase(glGetProgramiv, glGetProgramInfoLog, "link program failed: ", id) {}
GLE_ParamNotFound::GLE_ParamNotFound(const std::string& name): GLE_Error(std::string("parameter not found: ") + name) {}
GLE_InvalidArgument::GLE_InvalidArgument(const std::string& shname, const std::string& argname): GLE_Error(shname + ':' + argname) {}

// ---------------------- GLDevice ----------------------
ErrID::ErrID(int id, const char* msg): _id(id), _msg(msg) {}
const ErrID GLDevice::cs_err[5] = {
	ErrID(GL_INVALID_VALUE, "Numeric argument out of range"),
	ErrID(GL_INVALID_ENUM, "Enum argument out of range"),
	ErrID(GL_INVALID_OPERATION, "Operation illegal in current state"),
	ErrID(GL_INVALID_FRAMEBUFFER_OPERATION, "Framebuffer is incomplete"),
	ErrID(GL_OUT_OF_MEMORY, "Not enough memory left to execute command")
};

// ---------------------- GLShader ----------------------
void GLShader::_initShader() {
	_idSh = glCreateShader(_flag);

	const auto* pStr = _source.c_str();
	glShaderSource(_idSh, 1, &pStr, nullptr);
	glCompileShader(_idSh);

	// エラーが無かったか確認
	GLint compiled;
	glGetShaderiv(_idSh, GL_COMPILE_STATUS, &compiled);
	if(compiled == GL_FALSE)
		throw GLE_ShaderError(_idSh);
}

GLShader::GLShader() {}
GLShader::GLShader(GLuint flag, const std::string& src): _flag(flag), _source(src) {
	_initShader();
}
GLShader::~GLShader() {
	onDeviceLost();
}
bool GLShader::isEmpty() const {
	return _source.empty();
}
int GLShader::getShaderID() const {
	return _idSh;
}
void GLShader::onDeviceLost() {
	if(!isEmpty()) {
		glDeleteShader(_idSh);
		_idSh = 0;
	}
}
void GLShader::onDeviceReset() {
	if(!isEmpty())
		_initShader();
}

// ---------------------- GLProgram ----------------------
void GLProgram::_initProgram() {
	_idProg = glCreateProgram();
	for(int i=0 ; i<static_cast<int>(ShType::NUM_SHTYPE) ; i++) {
		auto& sh = _shader[i];
		// Geometryシェーダー以外は必須
		if(sh) {
			glAttachShader(_idProg, sh->getShaderID());
			GL_ACheck()
		} else {
			if(i != ShType::GEOMETRY)
				throw GLE_Error("missing shader elements (vertex or fragment)");
		}
	}

	glLinkProgram(_idProg);
	// エラーが無いかチェック
	int ib;
	glGetProgramiv(_idProg, GL_LINK_STATUS, &ib);
	if(ib == GL_FALSE)
		throw GLE_ProgramError(_idProg);
}
GLProgram::~GLProgram() {
	if(_idProg != 0)
		onDeviceLost();
}
void GLProgram::onDeviceLost() {
	for(auto& s : _shader) {
		if(s) {
			glDetachShader(_idProg, s->getShaderID());
			s->onDeviceLost();
		}
	}
	glDeleteProgram(_idProg);
	_idProg = 0;
}
void GLProgram::onDeviceReset() {
	for(auto& s : _shader)
		if(s)
			s->onDeviceReset();
}
const SPShader& GLProgram::getShader(ShType type) const {
	return _shader[(int)type];
}
int GLProgram::getUniformID(const std::string& name) const {
	GLint id = getUniformIDNc(name);
	if(id < 0)
		throw GLE_ParamNotFound(name);
	return id;
}
int GLProgram::getUniformIDNc(const std::string& name) const {
	return glGetUniformLocation(getProgramID(), name.c_str());
}
int GLProgram::getAttribID(const std::string& name) const {
	GLint id = getAttribIDNc(name);
	if(id < 0)
		throw GLE_ParamNotFound(name);
	return id;
}
GLuint GLProgram::getProgramID() const {
	return _idProg;
}
int GLProgram::getAttribIDNc(const std::string& name) const {
	return glGetAttribLocation(getProgramID(), name.c_str());
}
void GLProgram::use() const {
	glUseProgram(getProgramID());
	GL_ACheck()
}
