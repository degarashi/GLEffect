#include "glformat.hpp"

// ------------------------- GLFormat -------------------------
GLFormat::GLFormat(GLenum fmt): value(fmt) {}
bool GLFormat::Check(GLenum fmt, ID id) {
	auto itr = s_idMap.find(FmtID(static_cast<int>(id),fmt));
	return itr != s_idMap.end() && itr->second == id;
}
GLFormat::ID GLFormat::Detect(GLenum fmt, ID tag) {
	FmtID id(tag, fmt);
	auto itr = s_idMap.find(id);
	if(itr != s_idMap.end())
		return itr->second;
	return Invalid;
}

const GLFormatV::RetFormatV GLFormatV::cs_retV[] = {
	[](GLenum fmt) { return GLDepthFmt(fmt, nullptr); },
	[](GLenum fmt) { return GLStencilFmt(fmt, nullptr); },
	[](GLenum fmt) { return GLDSFmt(fmt, nullptr); },
	[](GLenum fmt) { return GLInFmt(fmt, nullptr); },
	[](GLenum fmt) { return GLInSizedFmt(fmt, nullptr); },
	[](GLenum fmt) { return GLInCompressedFmt(fmt, nullptr); },
	[](GLenum fmt) { return GLInRenderFmt(fmt, nullptr); },
	[](GLenum fmt) { return GLInReadFmt(fmt, nullptr); },
	[](GLenum fmt) { return GLTypeFmt(fmt, nullptr); }
};

GLFormatV GLFormatV::Detect(GLenum fmt) {
	return cs_retV[static_cast<int>(GLFormat::Detect(fmt, GLFormat::Tag_All))](fmt);
}

#define ADD_FMTID1(z, data, elem)	s_idMap.insert(std::make_pair(FmtID(static_cast<int>(data),elem), Invalid));
#define ADD_FMTID_ALL(z, data, elem)	s_idMap.insert(std::make_pair(FmtID(static_cast<int>(Tag_All),elem), data));
#define ADD_FMTID_DSC(z, data, elem)	s_idMap.insert(std::make_pair(FmtID(static_cast<int>(Tag_DSC),elem), data));
GLFormat::IDMap GLFormat::s_idMap(512);
void GLFormat::InitMap() {
	// フォーマット判定用エントリ
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID1, Internal, SEQ_INTERNAL)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID1, Internal_Sized, SEQ_INTERNALSIZED)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID1, Internal_Compressed, SEQ_INTERNALCOMPRESSED)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID1, Internal_Render, SEQ_INTERNALRENDER)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID1, Internal_Read, SEQ_INTERNALREAD)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID1, Type, SEQ_TYPE)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID1, DepthStencil, PSEQ_DSFORMAT)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID1, Depth, PSEQ_DEPTHFORMAT)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID1, Stencil, PSEQ_STENCILFORMAT)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID1, DepthStencil, PSEQ_DSFORMAT)
	// フォーマット検索用エントリ
	// より根本のクラスが優先される
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID_ALL, DepthStencil, PSEQ_DSFORMAT)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID_ALL, Stencil, PSEQ_STENCILFORMAT)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID_ALL, Depth, PSEQ_DEPTHFORMAT)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID_ALL, Type, SEQ_TYPE)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID_ALL, Internal_Read, SEQ_INTERNALREAD)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID_ALL, Internal_Render, SEQ_INTERNALRENDER)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID_ALL, Internal_Compressed, SEQ_INTERNALCOMPRESSED)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID_ALL, Internal_Sized, SEQ_INTERNALSIZED)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID_ALL, Internal, SEQ_INTERNAL)

	// Depth | Stencil | DS | Color の判別用
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID_DSC, Internal, SEQ_INTERNAL)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID_DSC, DepthStencil, PSEQ_DSFORMAT)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID_DSC, Stencil, PSEQ_STENCILFORMAT)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID_DSC, Depth, PSEQ_DEPTHFORMAT)
}
#undef ADD_FMTID1
#undef ADD_FMTID2
