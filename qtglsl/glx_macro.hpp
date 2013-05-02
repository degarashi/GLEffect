#pragma once
#define BOOST_PP_VARIADICS 1
#include <boost/preprocessor.hpp>

#define SEQ_VSEM BOOST_PP_REPEAT(NUM_TEXCOORD, PPFUNC_ADDNUM, TEXCOORD)(POSITION)(COLOR)(NORMAL)(BINORMAL)(TANGENT)
#define SEQ_GLTYPE (void)(bool)(int)(float)(vec2)(vec3)(vec4)(ivec2)(ivec3)(ivec4)(bvec2)(bvec3)(bvec4)(mat2)(mat3)(mat4)(sampler2D)(samplerCube)
#define SEQ_PRECISION (highp)(mediump)(lowp)
#define SEQ_INOUT (in)(out)(inout)
#define SEQ_BLOCK (attribute)(varying)(uniform)(const)
#define SEQ_GLSETTING ((linewidth,glLineWidth,float))((frontface,glFrontFace,int))((cullface,glCullFace,int))((depthrange,glDepthRange,float,float))((viewport,glViewport,int,int,int,int))\
		((scissor,glScissor,int,int,int,int))((samplecoverage,glSampleCoverage,float,bool))((stencilfunc,glStencilFunc,int,int,int))((stencilfuncfront,StencilFuncFront,int,int,int))\
		((stencilfuncback,StencilFuncBack,int,int,int))((stencilop,glStencilOp,int,int,int))((stencilopfront,StencilOpFront,int,int,int))((stencilopback,StencilOpBack,int,int,int))\
		((depthfunc,glDepthFunc,int))((blendeq,glBlendEquation,int))((blendeqca,glBlendEquationSeparate,int,int))((blendfunc,glBlendFunc,int,int))\
		((blendfuncca,glBlendFuncSeparate,int,int,int,int))((blendcolor,glBlendColor,float,float,float,float))\
		((colormask,glColorMask,bool,bool,bool,bool))((depthmask,glDepthMask,bool))\
		((stencilmask,glStencilMask,int))((stencilmaskfront,StencilMaskFront,int))((stencilmaskback,StencilMaskBack,int))

#define NUM_TEXCOORD 8
#define PPFUNC_ADDNUM(z,n,data) (BOOST_PP_CAT(data,n))
#define PPFUNC_STR(ign, data, elem) BOOST_PP_STRINGIZE(elem),
#define PPFUNC_ADD(ign, data, elem) (BOOST_PP_STRINGIZE(elem), BOOST_PP_CAT(elem, data))
#define PPFUNC_ENUM(ign, data, elem) BOOST_PP_CAT(elem, data),
#define EMPTY

#define PPFUNC_GLSET_ENUM(ign,data,elem) BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(0,elem), data),
#define PPFUNC_GLSET_ADD(ign,data,elem) (BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0,elem)), BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(0,elem),data))

// tupleのn番要素以降を列挙
#define ENUMTUPLE_FUNC(z,n,data) (BOOST_PP_TUPLE_ELEM(n,data)())
#define ENUMTUPLE(n,tup) BOOST_PP_SEQ_ENUM(BOOST_PP_REPEAT_FROM_TO(n, BOOST_PP_TUPLE_SIZE(tup), ENUMTUPLE_FUNC, tup))
#define PPFUNC_GLSET_FUNC(ign,data,elem) [](const ValueSettingR& vs) { vs.action(BOOST_PP_TUPLE_ELEM(1,elem), ENUMTUPLE(2,elem)); },

#define MAKE_SEQ(size, rel) MAKE_SEQ_D(size, rel)
#define MAKE_SEQ_D(size, rel) \
	BOOST_PP_CAT( \
		MAKE_SEQ_A_ ## size rel, \
		0X0 \
	)() \
	/**/

// size 2
#define MAKE_SEQ_A_2(x, y) ((x, y)) MAKE_SEQ_B_2
#define MAKE_SEQ_B_2(x, y) ((x, y)) MAKE_SEQ_A_2
// size 3
#define MAKE_SEQ_A_3(x, y, z) ((x, y, z)) MAKE_SEQ_B_3
#define MAKE_SEQ_B_3(x, y, z) ((x, y, z)) MAKE_SEQ_A_3

#define MAKE_SEQ_A_20X0()
#define MAKE_SEQ_B_20X0()
#define MAKE_SEQ_A_30X0()
#define MAKE_SEQ_B_30X0()

#define countof(elem) static_cast<int>(sizeof((elem))/sizeof((elem)[0]))
