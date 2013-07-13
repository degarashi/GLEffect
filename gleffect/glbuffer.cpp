#include "spinner/common.hpp"
#include "glresource.hpp"
#include <cstring>

// --------------------------- GLBuffer ---------------------------
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
		GL_ACheck()
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
		GL_ACheck();
	}
}
void GLBuffer::onDeviceReset() {
	_initBuffer();
}
GLBuffer::~GLBuffer() {
	onDeviceLost();
}

GLBuffer::GLBuffer(GLuint flag, GLuint dtype): _buffType(flag), _drawType(dtype), _stride(0), _idBuff(0) {}
GLuint GLBuffer::GetUnitSize(GLuint flag) {
	for(int i=0 ; i<countof(cs_cnv) ; i+=2) {
		if(cs_cnv[i] == flag)
			return cs_cnv[i+1];
	}
	throw GLE_Error("invalid unit size");
}

void GLBuffer::initData(const void* src, size_t nElem, GLuint stride) {
	if(stride != 0)
		_stride = stride;
	onDeviceLost();
	_buff.resize(nElem*_stride);
	std::memcpy(&_buff[0], src, nElem*_stride);
	onDeviceReset();
}
void GLBuffer::initData(ByteBuff&& buff, GLuint stride) {
	if(stride != 0)
		_stride = stride;
	onDeviceLost();
	_buff.swap(buff);
	onDeviceReset();
}
void GLBuffer::updateData(const void* src, size_t nElem, GLuint offset) {
	glBindBuffer(_buffType, _idBuff);
	glBufferSubData(_buffType, offset*_stride, nElem*_stride, src);
	glBindBuffer(_buffType, 0);
	GL_ACheck()
}

// --------------------------- GLVBuffer ---------------------------
GLVBuffer::GLVBuffer(GLuint dtype): GLBuffer(GL_ARRAY_BUFFER, dtype) {}

// --------------------------- GLIBuffer ---------------------------
GLIBuffer::GLIBuffer(GLuint dtype): GLBuffer(GL_ELEMENT_ARRAY_BUFFER, dtype) {}
void GLIBuffer::initData(const GLubyte* src, size_t nElem) {
	GLBuffer::initData(src, nElem, sizeof(GLubyte));
}
void GLIBuffer::initData(const GLushort* src, size_t nElem) {
	GLBuffer::initData(src, nElem, sizeof(GLushort));
}
void GLIBuffer::initData(ByteBuff&& buff) {
	GLBuffer::initData(std::forward<ByteBuff>(buff), sizeof(GLubyte));
}
void GLIBuffer::initData(const U16Buff& buff) {
	GLBuffer::initData(reinterpret_cast<const void*>(&buff[0]), buff.size(), sizeof(GLushort));
}
void GLIBuffer::updateData(const GLubyte* src, size_t nElem, GLuint offset) {
	GLBuffer::updateData(src, nElem, offset*sizeof(GLubyte));
}
void GLIBuffer::updateData(const GLushort* src, size_t nElem, GLuint offset) {
	GLBuffer::updateData(src, nElem, offset*sizeof(GLushort));
}

