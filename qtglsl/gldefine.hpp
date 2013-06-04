#pragma once
#include <memory>
#include <vector>

class GLEffect;
using SPEffect = std::shared_ptr<GLEffect>;
class VDecl;
using SPVDecl = std::shared_ptr<VDecl>;
class TPStructR;
class GLBuffer;
using SPBuffer = std::shared_ptr<GLBuffer>;
class GLVBuffer;
using SPVBuffer = std::shared_ptr<GLVBuffer>;
class GLIBuffer;
using SPIBuffer = std::shared_ptr<GLIBuffer>;
class IGLTexture;
using SPTexture = std::shared_ptr<IGLTexture>;
class GLProgram;
using SPProg = std::shared_ptr<GLProgram>;
class GLShader;
using SPShader = std::shared_ptr<GLShader>;

using ByteBuff = std::vector<uint8_t>;
using U16Buff = std::vector<uint16_t>;
