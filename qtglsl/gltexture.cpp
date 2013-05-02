#include "glresource.hpp"

GLuint GLTexture::getTextureID() const {
	return _idTex;
}
bool GLTexture::operator == (const GLTexture& t) const {
	return _idTex == t._idTex;
}
