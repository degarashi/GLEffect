#include "testgl.hpp"

GLRes::LHdl GLRes::_common(const QString& key, std::function<SPResource ()> cb) {
	std::string ps = key.toStdString();
	LHdl lh = getFromKey(ps);
	if(!lh.valid())
		lh = base_type::acquire(std::move(ps), cb()).first;
	return std::move(lh);
}
HLTex GLRes::loadTexture(const QString& path, bool bCube) {
	LHdl lh = _common(path, [&](){return SPResource(new TexFile(path,bCube));});
	return Cast<SPTexture>(std::move(lh));
}

HLSh GLRes::makeShader(GLuint flag, const std::string& src) {
	LHdl lh = base_type::acquire(SPResource(new GLShader(flag, src)));
	return Cast<SPShader>(std::move(lh));
}
HLFx GLRes::loadEffect(const QString& path) {
	LHdl lh = _common(path, [&](){ return SPResource(new GLEffect(path.toStdString())); });
	return Cast<SPEffect>(std::move(lh));
}
HLVb GLRes::makeVBuffer(GLuint dtype) {
	LHdl lh = base_type::acquire(SPResource(new GLVBuffer(dtype)));
	return Cast<SPVBuffer>(std::move(lh));
}
HLIb GLRes::makeIBuffer(GLuint dtype) {
	LHdl lh = base_type::acquire(SPResource(new GLIBuffer(dtype)));
	return Cast<SPIBuffer>(std::move(lh));
}

HLProg GLRes::makeProgram(HSh vsh, HSh psh) {
	LHdl lh = base_type::acquire(SPResource(new GLProgram(vsh,psh)));
	return Cast<SPProg>(std::move(lh));
}
HLProg GLRes::makeProgram(HSh vsh, HSh gsh, HSh psh) {
	LHdl lh = base_type::acquire(SPResource(new GLProgram(vsh,gsh,psh)));
	return Cast<SPProg>(std::move(lh));
}
