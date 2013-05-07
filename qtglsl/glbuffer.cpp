#include "glresource.hpp"

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
GLuint GLBuffer::getUnitSize() const { return _unitSize; }
GLuint GLBuffer::getBuffType() const { return _buffType; }
GLuint GLBuffer::getStride() const { return _unitSize; }
GLuint GLBuffer::getUnitFlag() const { return _unitFlag; }

void GLBuffer::use() const {
	glBindBuffer(_buffType, getBuffID());
	GLCheck()
}
void GLBuffer::_initBuffer() {
	glGenBuffers(1, &_idBuff);
	glBindBuffer(_buffType, _idBuff);
	glBufferData(_buffType, _buff.size(), &_buff[0], _drawType);
	glBindBuffer(_buffType, 0);
	GLCheck()
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

GLBuffer::GLBuffer(GLuint flag, GLuint dtype, GLuint unitFlag, GLuint stride):
		_buffType(flag), _drawType(dtype), _unitFlag(unitFlag),
		_unitSize(GetUnitSize(unitFlag)), _stride(stride)
{}
GLBuffer::GLBuffer(GLuint flag, GLuint dtype, GLuint unitFlag, GLuint stride, const ByteBuff& buff):
		GLBuffer(flag,dtype,unitFlag,stride)
{
	_buff = buff;
	_initBuffer();
}
GLBuffer::GLBuffer(GLuint flag, GLuint dtype, GLuint unitFlag, GLuint stride, ByteBuff&& buff):
		GLBuffer(flag,dtype,unitFlag,stride)
{
	_buff.swap(buff);
	_initBuffer();
}

GLuint GLBuffer::GetUnitSize(GLuint flag) {
	for(int i=0 ; i<countof(cs_cnv) ; i+=2) {
		if(cs_cnv[i] == flag)
			return cs_cnv[i+1];
	}
	throw GLE_Error("invalid unit size");
}