#pragma once
#include <memory>
#include <vector>
#include <GL/gl.h>
#include "glext.h"
#include <assert.h>

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
class GLFBuffer;
using UPFBuffer = std::unique_ptr<GLFBuffer>;
class GLRBuffer;
using UPRBuffer = std::unique_ptr<GLRBuffer>;

using ByteBuff = std::vector<uint8_t>;
using U16Buff = std::vector<uint16_t>;
using FloatBuff = std::vector<float>;
//! 複数のバッファ形式を1つに纏める
template <class T>
class AbstBuffer {
	using Buff = std::vector<T>;
	enum class Type {
		ConstPtr,
		Movable,
		Const,
		Invalid
	};
	Type _type;
	union {
		const T*	_pSrc;
		Buff*		_buffM;
		const Buff*	_buffC;
	};
	size_t	_size;

	void _invalidate() {
		_pSrc = nullptr;
		_type = Type::Invalid;
		_size = 0;
	}

	public:
		// 中身がMovableな時にちょっと微妙だけど後で考える
		AbstBuffer(const AbstBuffer& ab) {
			_type = ab._type;
			_pSrc = ab._pSrc;
			_size = ab._size;
		}
		//! initialize by const-pointer
		AbstBuffer(const void* src, size_t sz): _type(Type::ConstPtr), _size(sz), _pSrc(reinterpret_cast<const T*>(src)) {}
		//! initialize by movable-vector
		AbstBuffer(Buff&& buff): _type(Type::Movable), _buffM(&buff), _size(buff.size()) {}
		//! initialize by const-vector
		AbstBuffer(const Buff& buff): _type(Type::Const), _buffC(&buff), _size(buff.size()) {}

		void setTo(Buff& dst) {
			switch(_type) {
				case Type::ConstPtr:
					dst.assign(_pSrc, _pSrc+_size);
					break;
				case Type::Movable:
					dst = std::move(*_buffM);
					_invalidate();
					break;
				case Type::Const:
					dst = *_buffC;
					break;
				default:
					assert(false);
					break;
			}
		}
		Buff moveTo() {
			switch(_type) {
				case Type::ConstPtr: return Buff(_pSrc, _pSrc+_size);
				case Type::Movable: return std::move(*_buffM);
				case Type::Const: return *_buffC;
				default: assert(false);
			}
		}

		size_t getSize() const {
			return _size;
		}
		const T* getPtr() const {
			if(_type == Type::ConstPtr)
				return _pSrc;
			return &(*_buffC)[0];
		}
};
using AB_Byte = AbstBuffer<uint8_t>;
using AB_U16 = AbstBuffer<uint16_t>;
using AB_Float = AbstBuffer<float>;

enum ShType : unsigned int {
	VERTEX, GEOMETRY, PIXEL,
	NUM_SHTYPE
};
//! シェーダーIDに対するOpenGL定数
const static GLuint c_glShFlag[ShType::NUM_SHTYPE] = {
	GL_VERTEX_SHADER, GL_GEOMETRY_SHADER, GL_FRAGMENT_SHADER
};
