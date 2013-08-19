#include "spinner/common.hpp"
#include "glresource.hpp"
#include <cstring>

// --------------------------- GLBuffer ---------------------------
DEF_GLRESOURCE_CPP(GLBuffer)
GLuint GLBuffer::getBuffID() const { return _idBuff; }
GLuint GLBuffer::getBuffType() const { return _buffType; }
GLuint GLBuffer::getStride() const { return _stride; }

void GLBuffer::Use(GLBuffer& b) {
	glBindBuffer(b._buffType, b.getBuffID());
	GL_ACheck()
}
void GLBuffer::End(GLBuffer& b) {
	GL_ACheck()
	glBindBuffer(b._buffType, 0);
}

void GLBuffer::onDeviceLost() {
	if(_idBuff != 0) {
		glDeleteBuffers(1, &_idBuff);
		_idBuff = 0;
		GL_ACheck();
	}
}
void GLBuffer::onDeviceReset() {
	if(_idBuff == 0) {
		glGenBuffers(1, &_idBuff);
		if(!_buff.empty()) {
			auto u = use();
			glBufferData(_buffType, _buff.size(), &_buff[0], _drawType);
			u->end();
		}
	}
}
GLBuffer::~GLBuffer() {
	onDeviceLost();
}

GLBuffer::GLBuffer(GLuint flag, GLuint dtype): _buffType(flag), _drawType(dtype), _stride(0), _idBuff(0) {}
GLBuffer::Inner1& GLBuffer::initData(const void* src, size_t nElem, GLuint stride) {
	_stride = stride;
	_buff.resize(nElem*_stride);
	std::memcpy(&_buff[0], src, nElem*_stride);
	glBufferData(_buffType, _buff.size(), &_buff[0], _drawType);
	return Inner1::Cast(this);
}
GLBuffer::Inner1& GLBuffer::initData(ByteBuff&& buff, GLuint stride) {
	_stride = stride;
	_buff.swap(buff);
	glBufferData(_buffType, _buff.size(), &_buff[0], _drawType);
	return Inner1::Cast(this);
}
GLBuffer::Inner1& GLBuffer::updateData(const void* src, size_t nElem, GLuint offset) {
	glBufferSubData(_buffType, offset*_stride, nElem*_stride, src);
	return Inner1::Cast(this);
}

// --------------------------- GLVBuffer ---------------------------
GLVBuffer::GLVBuffer(GLuint dtype): GLBuffer(GL_ARRAY_BUFFER, dtype) {}

// --------------------------- GLIBuffer ---------------------------
DEF_GLRESOURCE_CPP(GLIBuffer)
GLIBuffer::GLIBuffer(GLuint dtype): GLBuffer(GL_ELEMENT_ARRAY_BUFFER, dtype) {}
GLIBuffer::Inner1& GLIBuffer::initData(const GLubyte* src, size_t nElem) {
	GLBuffer::initData(src, nElem, sizeof(GLubyte));
	return Inner1::Cast(this);
}
GLIBuffer::Inner1& GLIBuffer::initData(const GLushort* src, size_t nElem) {
	GLBuffer::initData(src, nElem, sizeof(GLushort));
	return Inner1::Cast(this);
}
GLIBuffer::Inner1& GLIBuffer::initData(ByteBuff&& buff) {
	GLBuffer::initData(std::forward<ByteBuff>(buff), sizeof(GLubyte));
	return Inner1::Cast(this);
}
GLIBuffer::Inner1& GLIBuffer::initData(const U16Buff& buff) {
	GLBuffer::initData(reinterpret_cast<const void*>(&buff[0]), buff.size(), sizeof(GLushort));
	return Inner1::Cast(this);
}
GLIBuffer::Inner1& GLIBuffer::updateData(const GLubyte* src, size_t nElem, GLuint offset) {
	GLBuffer::updateData(src, nElem, offset*sizeof(GLubyte));
	return Inner1::Cast(this);
}
GLIBuffer::Inner1& GLIBuffer::updateData(const GLushort* src, size_t nElem, GLuint offset) {
	GLBuffer::updateData(src, nElem, offset*sizeof(GLushort));
	return Inner1::Cast(this);
}

