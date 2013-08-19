#include "testgl.hpp"

GLRes::GLRes() {
	_bInit = false;
	_upFb.reset(new GLFBuffer());
}
GLRes::~GLRes() {
	onDeviceLost();
}

GLRes::LHdl GLRes::_common(const QString& key, std::function<UPResource ()> cb) {
	std::string ps = key.toStdString();
	LHdl lh = getFromKey(ps);
	if(!lh.valid())
		lh = base_type::acquire(std::move(ps), cb()).first;
	initHandle(lh);
	return std::move(lh);
}
HLTex GLRes::loadTexture(const QString& path, bool bCube) {
	LHdl lh = _common(path, [&](){return UPResource(new TexFile(path,bCube));});
	return Cast<UPTexture>(std::move(lh));
}

HLSh GLRes::makeShader(GLuint flag, const std::string& src) {
	LHdl lh = base_type::acquire(UPResource(new GLShader(flag, src)));
	initHandle(lh);
	return Cast<UPShader>(std::move(lh));
}
HLFx GLRes::loadEffect(const QString& path) {
	LHdl lh = _common(path, [&](){ return UPResource(new GLEffect(path.toStdString())); });
	return Cast<UPEffect>(std::move(lh));
}
HLVb GLRes::makeVBuffer(GLuint dtype) {
	LHdl lh = base_type::acquire(UPResource(new GLVBuffer(dtype)));
	initHandle(lh);
	return Cast<UPVBuffer>(std::move(lh));
}
HLIb GLRes::makeIBuffer(GLuint dtype) {
	LHdl lh = base_type::acquire(UPResource(new GLIBuffer(dtype)));
	initHandle(lh);
	return Cast<UPIBuffer>(std::move(lh));
}

HLProg GLRes::makeProgram(HSh vsh, HSh psh) {
	LHdl lh = base_type::acquire(UPResource(new GLProgram(vsh,psh)));
	initHandle(lh);
	return Cast<UPProg>(std::move(lh));
}
HLProg GLRes::makeProgram(HSh vsh, HSh gsh, HSh psh) {
	LHdl lh = base_type::acquire(UPResource(new GLProgram(vsh,gsh,psh)));
	initHandle(lh);
	return Cast<UPProg>(std::move(lh));
}
GLFBufferTmp& GLRes::getTmpFramebuffer() const {
	return *_tmpFb;
}

bool GLRes::deviceStatus() const {
	return _bInit;
}
void GLRes::onDeviceLost() {
	if(_bInit) {
		_bInit = false;
		for(auto& r : *this)
			r->onDeviceLost();
		_tmpFb.reset(nullptr);
		_upFb->onDeviceLost();
	}
}
void GLRes::onDeviceReset() {
	if(!_bInit) {
		_bInit = true;
		_upFb->onDeviceReset();
		_tmpFb.reset(new GLFBufferTmp(_upFb->getBufferID()));
		for(auto& r : *this)
			r->onDeviceReset();
	}
}
