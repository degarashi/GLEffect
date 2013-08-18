#include "testgl.hpp"

GLRes::GLRes() {
	_upFb.reset(new GLFBuffer());
	_tmpFb.reset(new GLFBufferTmp(_upFb->getBufferID()));
	onDeviceReset();
}
GLRes::~GLRes() {
	onDeviceLost();
}

GLRes::LHdl GLRes::_common(const QString& key, std::function<UPResource ()> cb) {
	std::string ps = key.toStdString();
	LHdl lh = getFromKey(ps);
	if(!lh.valid())
		lh = base_type::acquire(std::move(ps), cb()).first;
	return std::move(lh);
}
HLTex GLRes::loadTexture(const QString& path, bool bCube) {
	LHdl lh = _common(path, [&](){return UPResource(new TexFile(path,bCube));});
	return Cast<UPTexture>(std::move(lh));
}

HLSh GLRes::makeShader(GLuint flag, const std::string& src) {
	LHdl lh = base_type::acquire(UPResource(new GLShader(flag, src)));
	return Cast<UPShader>(std::move(lh));
}
HLFx GLRes::loadEffect(const QString& path) {
	LHdl lh = _common(path, [&](){ return UPResource(new GLEffect(path.toStdString())); });
	return Cast<UPEffect>(std::move(lh));
}
HLVb GLRes::makeVBuffer(GLuint dtype) {
	LHdl lh = base_type::acquire(UPResource(new GLVBuffer(dtype)));
	return Cast<UPVBuffer>(std::move(lh));
}
HLIb GLRes::makeIBuffer(GLuint dtype) {
	LHdl lh = base_type::acquire(UPResource(new GLIBuffer(dtype)));
	return Cast<UPIBuffer>(std::move(lh));
}

HLProg GLRes::makeProgram(HSh vsh, HSh psh) {
	LHdl lh = base_type::acquire(UPResource(new GLProgram(vsh,psh)));
	return Cast<UPProg>(std::move(lh));
}
HLProg GLRes::makeProgram(HSh vsh, HSh gsh, HSh psh) {
	LHdl lh = base_type::acquire(UPResource(new GLProgram(vsh,gsh,psh)));
	return Cast<UPProg>(std::move(lh));
}
GLFBufferTmp& GLRes::getTmpFramebuffer() const {
	return *_tmpFb;
}

void GLRes::onDeviceLost() {
	for(auto& r : *this)
		r->onDeviceLost();
	_upFb->onDeviceLost();
}
void GLRes::onDeviceReset() {
	_upFb->onDeviceReset();
	for(auto& r : *this)
		r->onDeviceReset();
}
