#include <GL/gl.h>
#include <GL/glu.h>
#include "glresource.hpp"

// ------------------------- IGLTexture -------------------------
bool IGLTexture::_onDeviceReset() {
	if(_idTex == 0) {
		glGenTextures(1, &_idTex);
		use();
		GL_ACheck()
		return true;
	}
	return false;
}
void IGLTexture::_applyFilter() {
	use();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _filMag);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _filMin);
	use_end();
	GL_ACheck()
}

IGLTexture::IGLTexture(GLuint filMin, GLuint filMag): _idTex(0), _filMin(filMin), _filMag(filMag) {}
IGLTexture::~IGLTexture() { onDeviceLost(); }
int IGLTexture::getWidth() const { return _width; }
int IGLTexture::getHeight() const { return _height; }
GLint IGLTexture::getTextureID() const { return _idTex; }
GLuint IGLTexture::getFilterMin() const { return _filMin; }
GLuint IGLTexture::getFilterMag() const { return _filMag; }

void IGLTexture::setFilter(GLuint fMin, GLuint fMag) {
	_filMin = fMin;
	_filMag = fMag;
	_applyFilter();
}
void IGLTexture::onDeviceLost() {
	if(_idTex != 0) {
		glDeleteTextures(1, &_idTex);
		_idTex = 0;
		_width = _height = 0;
		GL_ACheck()
	}
}
void IGLTexture::use() const {
	glBindTexture(GL_TEXTURE_2D, _idTex);
	GL_ACheck()
}
void IGLTexture::use(int n) const {
	glActiveTexture(GL_TEXTURE0 + n);
	use();
}
void IGLTexture::use_end() {
	glBindTexture(GL_TEXTURE_2D, 0);
}

// ------------------------- TexFile -------------------------
TexFile::TexFile(const QString& path, GLuint filMin, GLuint filMag): IGLTexture(filMin, filMag), _fPath(path) {
	onDeviceReset();
}
void TexFile::onDeviceReset() {
	if(_onDeviceReset()) {
		QImage img(_fPath);
		auto res = gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, img.width(), img.height(), GL_BGRA, GL_UNSIGNED_BYTE, img.constBits());
		if(res != 0)
			throw GLE_Error(reinterpret_cast<const char*>(gluErrorString(res)));
		_width = img.width();
		_height = img.height();
		_applyFilter();
	}
}
bool TexFile::operator == (const TexFile& t) const {
	return _fPath == t._fPath;
}

// ------------------------- TexUser -------------------------
bool TexUser::operator == (const TexUser& t) const {
	return _image == t._image;
}
void TexUser::onDeviceReset() {
	if(_onDeviceReset()) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _image.width(), _image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, _image.constBits());
		GL_ACheck()
		_width = _image.width();
		_height = _image.height();
		_applyFilter();
	}
}
