#include <GL/gl.h>
#include <GL/glu.h>
#include "glresource.hpp"

// ------------------------- IGLTexture -------------------------
const GLuint IGLTexture::cs_Filter[3][2] = {
	{GL_NEAREST, GL_LINEAR},
	{GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR},
	{GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR}
};
IGLTexture::IGLTexture(bool bCube):
	_idTex(0), _iLinearMag(0), _iLinearMin(0), _iWrapS(GL_CLAMP_TO_EDGE), _iWrapT(GL_CLAMP_TO_EDGE),
	_bChanged(true), _state(State::NotDecided), _texFlag(bCube ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D), _coeff(0) {}

bool IGLTexture::_onDeviceReset() {
	if(_idTex == 0) {
		glGenTextures(1, &_idTex);
		glBindTexture(_texFlag, _idTex);
		return true;
	}
	return false;
}
void IGLTexture::_applyFilter() {
	if((_state==State::NotDecided && !(_state=NoMipmap)) || _bChanged) {
		_bChanged = false;
		glTexParameteri(_texFlag, GL_TEXTURE_MAG_FILTER, cs_Filter[0][_iLinearMag]);
		glTexParameteri(_texFlag, GL_TEXTURE_MIN_FILTER, cs_Filter[_state][_iLinearMin]);
		glTexParameteri(_texFlag, GL_TEXTURE_WRAP_S, _iWrapS);
		glTexParameteri(_texFlag, GL_TEXTURE_WRAP_T, _iWrapT);

		GLfloat aMax;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aMax);
		glTexParameteri(_texFlag, GL_TEXTURE_MAX_ANISOTROPY_EXT, aMax*_coeff);

		GL_ACheck()
	}
}
void IGLTexture::applyFilter() {
	use();
	_applyFilter();
	use_end();
}

IGLTexture::~IGLTexture() { onDeviceLost(); }
int IGLTexture::getWidth() const { return _width; }
int IGLTexture::getHeight() const { return _height; }
GLint IGLTexture::getTextureID() const { return _idTex; }
bool IGLTexture::isMipmap() const { return _state==MipmapNear || _state==MipmapLinear; }
bool IGLTexture::isCubemap() const { return _texFlag != GL_TEXTURE_2D; }

void IGLTexture::setAnisotropicCoeff(float coeff) {
	_coeff = coeff;
	_bChanged = true;
}
void IGLTexture::setFilter(bool bLinearMag, bool bLinearMin) {
	_iLinearMag = bLinearMag ? 1 : 0;
	_iLinearMin = bLinearMin ? 1 : 0;
	_bChanged = true;
}
void IGLTexture::setMipmap(State level) {
	if(_state == NotDecided || _state != level) {
		_state = level;
		onDeviceLost();
		onDeviceReset();
	}
}

void IGLTexture::onDeviceLost() {
	if(_idTex != 0) {
		glDeleteTextures(1, &_idTex);
		_idTex = 0;
		_width = _height = 0;
		GL_ACheck()
	}
}
void IGLTexture::use() {
	glBindTexture(_texFlag, _idTex);
	_applyFilter();
	GL_ACheck()
}
void IGLTexture::use(int n) {
	glActiveTexture(GL_TEXTURE0 + n);
	use();
}
void IGLTexture::use_end() const {
	glBindTexture(_texFlag, 0);
}
void IGLTexture::setUVWrap(GLuint s, GLuint t) {
	_iWrapS = s;
	_iWrapT = t;
	_bChanged = true;
}


#include <functional>
namespace {
	using ImgSize = std::pair<int,int>;
	auto GLTNomipBase = [](GLuint flag, int w, int h, const GLubyte* ptr) -> ImgSize {
		glTexImage2D(flag, 0, GL_RGBA, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, ptr);
		return ImgSize(w,h);
	};
	auto GLTMipBase = [](GLuint flag, int w, int h, const GLubyte* ptr) -> ImgSize {
		auto res = gluBuild2DMipmaps(flag, GL_RGBA, w, h, GL_BGRA, GL_UNSIGNED_BYTE, ptr);
		if(res != 0)
			throw GLE_Error(reinterpret_cast<const char*>(gluErrorString(res)));
		return ImgSize(w,h);
	};

	// Mip / NoMip を吸収
	auto GLTNomipI = [](GLuint flag, const QImage& img) -> ImgSize {
		return GLTNomipBase(flag, img.width(), img.height(), img.constBits()); };
	auto GLTMipI = [](GLuint flag, const QImage& img) -> ImgSize {
		return GLTMipBase(flag, img.width(), img.height(), img.constBits()); };

	auto GLTNomip = [](GLuint flag, const QString& str) -> ImgSize {
		QImage img(str);
		return GLTNomipI(flag, img); };
	auto GLTMip = [](GLuint flag, const QString& str) -> ImgSize {
			QImage img(str);
			return GLTMipI(flag, img); };

	auto GLTNomipG = [](GLuint flag, const ITDGen* gen) -> ImgSize {
		return GLTNomipBase(flag, gen->getWidth(), gen->getHeight(), gen->getPtr()); };
	auto GLTMipG = [](GLuint flag, const ITDGen* gen) -> ImgSize {
		return GLTMipBase(flag, gen->getWidth(), gen->getHeight(), gen->getPtr()); };

	struct MyCompare : boost::static_visitor<bool> {
		template <class T>
		bool operator()(const T& t0, const T& t1) const { return t0 == t1; }
		template <class T0, class T1>
		bool operator()(const T0&, const T1&) const { return false; }
	};

	// Cube / 2D を吸収
	template <class PROC>
	struct CubeProc : boost::static_visitor<ImgSize> {
		PROC _proc;
		CubeProc(PROC proc): _proc(proc) {}

		template <class T>
		ImgSize operator()(const T& t) const {
			return _proc(GL_TEXTURE_2D, t);
		}
		template <class T, int N>
		ImgSize operator()(const Pack<T,N>& pck) const {
			ImgSize ret;
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
TexFile::TexFile(const QString& path, bool bCube): IGLTexture(bCube), _fPath(path) {
	onDeviceReset();
}
TexFile::TexFile(const QString& path0, const QString& path1, const QString& path2,
				 const QString& path3, const QString& path4, const QString& path5): IGLTexture(true), _fPath(QS6{path0,path1,path2,path3,path4,path5}) {}

void TexFile::onDeviceReset() {
	if(_onDeviceReset()) {
		ImgSize size = boost::apply_visitor(MakeCubeProc(isMipmap() ? GLTMip : GLTNomip), _fPath);
		_width = size.first;
		_height = size.second;
		// サーフェスを作りなおしたので更新フラグを立てる
		_bChanged = true;
		GL_ACheck()
	}
}
bool TexFile::operator == (const TexFile& t) const {
	return boost::apply_visitor(MyCompare(), _fPath, t._fPath);
}

// ------------------------- TexUser -------------------------
TexUser::TexUser(const QImage& img): IGLTexture(false), _image(img) {}
TexUser::TexUser(const QImage& img0, const QImage& img1, const QImage& img2,
				 const QImage& img3, const QImage& img4, const QImage& img5): IGLTexture(true), _image(QI6{img0,img1,img2,img3,img4,img5}) {}

void TexUser::onDeviceReset() {
	if(_onDeviceReset()) {
		ImgSize size = boost::apply_visitor(MakeCubeProc(isMipmap() ? GLTMipI : GLTNomipI), _image);
		_width = size.first;
		_height = size.second;
		_bChanged = true;
		GL_ACheck()
	}
}
bool TexUser::operator == (const TexUser& t) const {
	return boost::apply_visitor(MyCompare(), _image, t._image);
}

// ------------------------- TexDebug -------------------------
TexDebug::TexDebug(ITDGen* gen, bool bCube): IGLTexture(bCube), _upGen(gen) {
	if(bCube) _gen = ITD6(gen);
	else _gen = gen;
	onDeviceReset();
}
void TexDebug::onDeviceReset() {
	if(_onDeviceReset()) {
		ImgSize size = boost::apply_visitor(MakeCubeProc(isMipmap() ? GLTMipG : GLTNomipG), _gen);
		_width = size.first;
		_height = size.second;
		_bChanged = true;
		GL_ACheck()
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
