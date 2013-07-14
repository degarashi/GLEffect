#pragma once
#include <memory>
#include <vector>
#include <GL/gl.h>

struct IGLResource;
using UPResource = std::unique_ptr<IGLResource>;
class GLEffect;
using UPEffect = std::unique_ptr<GLEffect>;
class VDecl;
using UPVDecl = std::unique_ptr<VDecl>;
class TPStructR;
class GLBuffer;
using UPBuffer = std::unique_ptr<GLBuffer>;
class GLVBuffer;
using UPVBuffer = std::unique_ptr<GLVBuffer>;
class GLIBuffer;
using UPIBuffer = std::unique_ptr<GLIBuffer>;
class IGLTexture;
using UPTexture = std::unique_ptr<IGLTexture>;
class GLProgram;
using UPProg = std::unique_ptr<GLProgram>;
class GLShader;
using UPShader = std::unique_ptr<GLShader>;

using ByteBuff = std::vector<uint8_t>;
using U16Buff = std::vector<uint16_t>;

enum ShType : unsigned int {
	VERTEX, GEOMETRY, PIXEL,
	NUM_SHTYPE
};
//! シェーダーIDに対するOpenGL定数
const static GLuint c_glShFlag[ShType::NUM_SHTYPE] = {
	GL_VERTEX_SHADER, GL_GEOMETRY_SHADER, GL_FRAGMENT_SHADER
};
