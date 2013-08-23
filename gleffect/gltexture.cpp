#include <GL/gl.h>
#include <GL/glu.h>
#include "glresource.hpp"

// ------------------------- IGLTexture -------------------------
DEF_GLRESOURCE_CPP(IGLTexture)
const GLuint IGLTexture::cs_Filter[3][2] = {
	{GL_NEAREST, GL_LINEAR},
	{GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR},
	{GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR}
};
IGLTexture::IGLTexture(GLInSizedFmt fmt, bool bCube):
	_idTex(0), _iLinearMag(0), _iLinearMin(0), _iWrapS(GL_CLAMP_TO_EDGE), _iWrapT(GL_CLAMP_TO_EDGE),
	_actID(0), _mipLevel(NoMipmap), _texFlag(bCube ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D), _format(fmt), _coeff(0) {}

void IGLTexture::Use(IGLTexture& t) {
	glActiveTexture(GL_TEXTURE0 + t._actID);
	glBindTexture(t._texFlag, t._idTex);
}
void IGLTexture::End(IGLTexture& t) {
	GL_ACheck()
	glBindTexture(t._texFlag, 0);
}

bool IGLTexture::_onDeviceReset() {
	if(_idTex == 0) {
		glGenTextures(1, &_idTex);
		// フィルタの設定
		use()->setFilter(_mipLevel, static_cast<bool>(_iLinearMag), static_cast<bool>(_iLinearMin))
			->setAnisotropicCoeff(_coeff)
			->setUVWrap(_iWrapS, _iWrapT);
		return true;
	}
	return false;
}

IGLTexture::~IGLTexture() { onDeviceLost(); }
const Size& IGLTexture::getSize() const { return _size; }
GLint IGLTexture::getTextureID() const { return _idTex; }
void IGLTexture::setActiveID(GLuint n) { _actID = n; }
bool IGLTexture::isMipmap() const { return  IsMipmap(_mipLevel); }
bool IGLTexture::isCubemap() const { return _texFlag != GL_TEXTURE_2D; }

bool IGLTexture::IsMipmap(State level) {
	return level >= MipmapNear;
}
IGLTexture::Inner1& IGLTexture::setAnisotropicCoeff(float coeff) {
	_coeff = coeff;
	GLfloat aMax;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aMax);
	glTexParameteri(_texFlag, GL_TEXTURE_MAX_ANISOTROPY_EXT, aMax*_coeff);
	return Inner1::Cast(this);
}
IGLTexture::Inner1& IGLTexture::setFilter(State miplevel, bool bLinearMag, bool bLinearMin) {
	_iLinearMag = bLinearMag ? 1 : 0;
	_iLinearMin = bLinearMin ? 1 : 0;
	bool b = isMipmap() ^ IsMipmap(miplevel);
	_mipLevel = miplevel;
	if(b) {
		// ミップマップの有りなしを切り替える時はテクスチャを作りなおす
		End(*this);
		onDeviceLost();
		onDeviceReset();
		Use(*this);
	}
	glTexParameteri(_texFlag, GL_TEXTURE_MAG_FILTER, cs_Filter[0][_iLinearMag]);
	glTexParameteri(_texFlag, GL_TEXTURE_MIN_FILTER, cs_Filter[_mipLevel][_iLinearMin]);

	return Inner1::Cast(this);
}

void IGLTexture::onDeviceLost() {
	if(_idTex != 0) {
		glDeleteTextures(1, &_idTex);
		_idTex = 0;
		_size = Size(0,0);
		GL_ACheck()
	}
}
IGLTexture::Inner1& IGLTexture::setUVWrap(GLuint s, GLuint t) {
	_iWrapS = s;
	_iWrapT = t;

	glTexParameteri(_texFlag, GL_TEXTURE_WRAP_S, _iWrapS);
	glTexParameteri(_texFlag, GL_TEXTURE_WRAP_T, _iWrapT);
	return Inner1::Cast(this);
}

#include <functional>
namespace {
	auto GLTNomipBase = [](GLuint flag, int w, int h, const GLubyte* ptr) -> Size {
		glTexImage2D(flag, 0, GL_RGBA, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, ptr);
		return Size(w,h);
	};
	auto GLTMipBase = [](GLuint flag, int w, int h, const GLubyte* ptr) -> Size {
		auto res = gluBuild2DMipmaps(flag, GL_RGBA, w, h, GL_BGRA, GL_UNSIGNED_BYTE, ptr);
		if(res != 0)
			throw GLE_Error(reinterpret_cast<const char*>(gluErrorString(res)));
		return Size(w,h);
	};

	// Mip / NoMip を吸収
	auto GLTNomipI = [](GLuint flag, const QImage& img) -> Size {
		return GLTNomipBase(flag, img.width(), img.height(), img.constBits()); };
	auto GLTMipI = [](GLuint flag, const QImage& img) -> Size {
		return GLTMipBase(flag, img.width(), img.height(), img.constBits()); };

	auto GLTNomip = [](GLuint flag, const QString& str) -> Size {
		QImage img(str);
		return GLTNomipI(flag, img); };
	auto GLTMip = [](GLuint flag, const QString& str) -> Size {
			QImage img(str);
			return GLTMipI(flag, img); };

	auto GLTNomipG = [](GLuint flag, const ITDGen* gen) -> Size {
		return GLTNomipBase(flag, gen->getWidth(), gen->getHeight(), gen->getPtr()); };
	auto GLTMipG = [](GLuint flag, const ITDGen* gen) -> Size {
		return GLTMipBase(flag, gen->getWidth(), gen->getHeight(), gen->getPtr()); };

	struct MyCompare : boost::static_visitor<bool> {
		template <class T>
		bool operator()(const T& t0, const T& t1) const { return t0 == t1; }
		template <class T0, class T1>
		bool operator()(const T0&, const T1&) const { return false; }
	};

	// Cube / 2D を吸収
	template <class PROC>
	struct CubeProc : boost::static_visitor<Size> {
		PROC _proc;
		CubeProc(PROC proc): _proc(proc) {}

		template <class T>
		Size operator()(const T& t) const {
			return _proc(GL_TEXTURE_2D, t);
		}
		template <class T, int N>
		Size operator()(const Pack<T,N>& pck) const {
			Size ret;
			for(int i=0 ; i<6 ; i++)
				ret = _proc(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, pck.val[i]);
			return ret;
		}
	};
	template <class PROC>
	CubeProc<PROC> MakeCubeProc(PROC proc) {
		return CubeProc<PROC>(proc);
	}
}
// ------------------------- TexFile -------------------------
TexFile::TexFile(const QString& path, bool bCube): IGLTexture(GL_RGBA8, bCube), _fPath(path) {}
TexFile::TexFile(const QString& path0, const QString& path1, const QString& path2,
				 const QString& path3, const QString& path4, const QString& path5): IGLTexture(GL_RGBA8, true), _fPath(QS6{path0,path1,path2,path3,path4,path5}) {}

void TexFile::onDeviceReset() {
	if(_onDeviceReset()) {
		auto u = use();
		_size = boost::apply_visitor(MakeCubeProc(isMipmap() ? GLTMip : GLTNomip), _fPath);
		u->end();
	}
}
bool TexFile::operator == (const TexFile& t) const {
	return boost::apply_visitor(MyCompare(), _fPath, t._fPath);
}

// ------------------------- TexUser -------------------------
TexUser::TexUser(const QImage& img): IGLTexture(GL_RGBA8, false), _image(img) {}
TexUser::TexUser(const QImage& img0, const QImage& img1, const QImage& img2,
				 const QImage& img3, const QImage& img4, const QImage& img5): IGLTexture(GL_RGBA8, true), _image(QI6{img0,img1,img2,img3,img4,img5}) {}

void TexUser::onDeviceReset() {
	if(_onDeviceReset()) {
		auto u = use();
		_size = boost::apply_visitor(MakeCubeProc(isMipmap() ? GLTMipI : GLTNomipI), _image);
		u->end();
	}
}
bool TexUser::operator == (const TexUser& t) const {
	return boost::apply_visitor(MyCompare(), _image, t._image);
}
// ------------------------- TexEmpty -------------------------
TexEmpty::TexEmpty(const Size& size, GLInSizedFmt fmt, bool bRestore): IGLTexture(fmt, false) {
	_size = size;
	if(bRestore) {
		// DeviceLost時に内容を復元するタイプならバッファを確保しておく
		auto& info = *GLFormat::QueryInfo(fmt);
		_buff = ByteBuff(size.width * size.height * GLFormat::QuerySize(info.toType) * info.numType);
	}
}
void TexEmpty::onDeviceLost() {
	if(_idTex != 0) {
		if(_buff) {
			auto& info = *GLFormat::QueryInfo(_format);
#ifdef USE_OPENGLES2
			//	OpenGL ES2ではglTexImage2Dが実装されていないのでFramebufferにセットしてglReadPixelsで取得
			GLFBufferTmp& tmp = mgr_gl.getTmpFramebuffer();
			auto u = *tmp;
			u.attachColor(0, _idTex);
			glReadPixels(0, 0, _size.width, _size.height, info.toBase, info.toType, &_buff->operator[](0));
			u.attachColor(0, 0);
#else
			glGetTexImage(GL_TEXTURE_2D, 0, info.toBase, info.toType, &_buff->operator[](0));
#endif
		}
		IGLTexture::onDeviceLost();
	}
}
void TexEmpty::onDeviceReset() {
	if(_onDeviceReset()) {
		if(_buff) {
			// バッファの内容から復元
			auto u = use();
			auto& info = *GLFormat::QueryInfo(_format);
			glTexImage2D(GL_TEXTURE_2D, 0, _format.get(), _size.width, _size.height, 0, _format.get(), info.toType, &_buff->operator [](0));
			u->end();
		}
	}
}
bool TexEmpty::operator == (const TexEmpty& t) const {
	return getTextureID() == t.getTextureID();
}

// ------------------------- TexDebug -------------------------
TexDebug::TexDebug(ITDGen* gen, bool bCube): IGLTexture(GL_RGBA8, bCube), _upGen(gen) {
	if(bCube) _gen = ITD6(gen);
	else _gen = gen;
}
void TexDebug::onDeviceReset() {
	if(_onDeviceReset()) {
		auto u = use();
		_size = boost::apply_visitor(MakeCubeProc(isMipmap() ? GLTMipG : GLTNomipG), _gen);
		u->end();
	}
}
bool TexDebug::operator == (const TexDebug& t) const {
	return boost::apply_visitor(MyCompare(), _gen, t._gen);
}

// ------------------------- ITDGen -------------------------
ITDGen::ITDGen(int w, int h): _width(w), _height(h) {}
const GLubyte* ITDGen::getPtr() const { return _buff.get(); }
int ITDGen::getWidth() const { return _width; }
int ITDGen::getHeight() const { return _height; }

struct Bresen {
	int _width, _tgt;
	int _cur, _error;

	Bresen(int width, int tgt): _width(width), _tgt(tgt), _cur(0), _error(0/*width/2*/) {}
	int current() const { return _cur; }
	void advance() {
		_error += _tgt;
		if(_error >= _width) {
			_error -= _width;
			++_cur;
		}
	}
};

// ------------------------- TDChecker -------------------------
TDChecker::TDChecker(const spn::Vec4& col0, const spn::Vec4& col1, int nDivW, int nDivH, int w, int h): ITDGen(w,h) {
	GLubyte pack[2][4];
	const spn::Vec4* (col[2]) = {&col0, &col1};
	for(int k=0 ; k<2 ; k++) {
		for(int i=0 ; i<4 ; i++)
			pack[k][i] = static_cast<GLubyte>((*col)[k].m[i] * 255 + 0.5f);
	}

	// データの生成
	Bresen brY(h, nDivH);
	GLuint* ptr = new GLuint[w*h];
	_buff.reset(reinterpret_cast<GLubyte*>(ptr));
	for(int i=0 ; i<h ; i++) {
		int y = brY.current();
		Bresen brX(w, nDivW);
		for(int j=0 ; j<w ; j++) {
			int x = brX.current();
			*ptr++ = *reinterpret_cast<GLuint*>(pack[(x^y) & 1]);
			brX.advance();
		}
		brY.advance();
	}
}

// ------------------------- TDCChecker -------------------------
TDCChecker::TDCChecker(int, int, int w, int h): ITDGen(w, h) {
	const static GLubyte tex[] = {
		255, 255, 255, 255,     0,   0,   0, 255,   255, 255, 255 ,255,     0,   0,   0, 255,
		255,   0,   0, 255,     0, 255,   0, 255,     0,   0, 255 ,255,   255, 255, 255, 255,
		128,   0,   0, 255,     0, 128,   0, 255,     0,   0, 128 ,255,   128, 128, 128, 255,
		255, 255,   0, 255,   255,   0, 255, 255,     0, 255, 255 ,255,   255, 255, 255, 255,
	};
	auto* ptr = new GLubyte[sizeof(tex)];
	memcpy(ptr, tex, sizeof(tex));
	_buff.reset(ptr);
}

// ------------------------- TDBorder -------------------------
TDBorder::TDBorder(const spn::Vec4&, const spn::Vec4&, int w, int h): ITDGen(w,h) {
	throw std::runtime_error("not implemented yet");
}
