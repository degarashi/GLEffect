#include "glresource.hpp"
#include <cstring>

const GLuint GLBuffer::cs_cnv[] = {
	GL_FLOAT, 4,
	GL_DOUBLE, 8,
	GL_BYTE, 1,
	GL_UNSIGNED_BYTE, 1,
	GL_SHORT, 2,
	GL_UNSIGNED_SHORT, 2,
	GL_INT, 4,
	GL_UNSIGNED_INT, 4,
	GL_HALF_FLOAT, 2,
	GL_INT_2_10_10_10_REV, 4,
	GL_UNSIGNED_INT_2_10_10_10_REV, 4
};

GLuint GLBuffer::getBuffID() const { return _idBuff; }
GLuint GLBuffer::getBuffType() const { return _buffType; }
GLuint GLBuffer::getStride() const { return _stride; }

void GLBuffer::use() const {
	glBindBuffer(_buffType, getBuffID());
	GL_ACheck()
}
void GLBuffer::_initBuffer() {
	if(!_buff.empty()) {
		glGenBuffers(1, &_idBuff);
		glBindBuffer(_buffType, _idBuff);
		glBufferData(_buffType, _buff.size(), &_buff[0], _drawType);
		glBindBuffer(_buffType, 0);
		GL_ACheck()
	}
}

void GLBuffer::onDeviceLost() {
	if(_idBuff != 0) {
		glDeleteBuffers(1, &_idBuff);
		_idBuff = 0;
	}
}
void GLBuffer::onDeviceReset() {
	_initBuffer();
}
GLBuffer::~GLBuffer() {
	onDeviceLost();
}

GLBuffer::GLBuffer(GLuint flag, GLuint dtype, GLuint stride): _buffType(flag), _drawType(dtype), _stride(stride) {}
GLBuffer::GLBuffer(GLuint flag, GLuint dtype, GLuint stride, const void* src, size_t length): GLBuffer(flag,dtype,stride) {
	setBufferData(src, length);
}
GLBuffer::GLBuffer(GLuint flag, GLuint dtype, GLuint stride, ByteBuff&& buff): GLBuffer(flag,dtype,stride) {
	setBufferData(std::forward<ByteBuff>(buff));
}

GLuint GLBuffer::GetUnitSize(GLuint flag) {
	for(int i=0 ; i<countof(cs_cnv) ; i+=2) {
		if(cs_cnv[i] == flag)
			return cs_cnv[i+1];
	}
	throw GLE_Error("invalid unit size");
}

void GLBuffer::setBufferData(const void* src, size_t length) {
	onDeviceLost();
	_buff.resize(length);
	std::memcpy(&_buff[0], src, length);
	onDeviceReset();
}
void GLBuffer::setBufferData(ByteBuff&& buff) {
	onDeviceLost();
	_buff.swap(buff);
	onDeviceReset();
}
