#include "glformat.hpp"

// ------------------------- GLFormat -------------------------
GLFormat::GLFormat(GLenum fmt): value(fmt) {}
bool GLFormat::Check(GLenum fmt, ID id) {
	auto itr = s_idMap.find(FmtID(static_cast<int>(id),fmt));
	return itr != s_idMap.end() && itr->second == id;
}
GLFormat::ID GLFormat::Detect(GLenum fmt) {
	auto num = static_cast<int>(ID::NumID);
	FmtID id(num, fmt);
	auto itr = s_idMap.find(id);
	if(itr != s_idMap.end())
		return itr->second;
	return ID::Invalid;
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
	return cs_retV[static_cast<int>(GLFormat::Detect(fmt))](fmt);
}


#define ADD_FMTID1(z, data, elem)	s_idMap.insert(std::make_pair(FmtID(static_cast<int>(data),elem), GLFormat::ID::NumID));
#define ADD_FMTID2(z, data, elem)	s_idMap.insert(std::make_pair(FmtID(0,elem), data));
GLFormat::IDMap GLFormat::s_idMap(256);
void GLFormat::InitMap() {
	// フォーマット判定用エントリ
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID1, ID::Internal, SEQ_INTERNAL)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID1, ID::Internal_Sized, SEQ_INTERNALSIZED)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID1, ID::Internal_Compressed, SEQ_INTERNALCOMPRESSED)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID1, ID::Internal_Render, SEQ_INTERNALRENDER)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID1, ID::Internal_Read, SEQ_INTERNALREAD)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID1, ID::Type, SEQ_TYPE)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID1, ID::DepthStencil, PSEQ_DSFORMAT)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID1, ID::Depth, PSEQ_DEPTHFORMAT)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID1, ID::Stencil, PSEQ_STENCILFORMAT)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID1, ID::DepthStencil, PSEQ_DSFORMAT)
	// フォーマット検索用エントリ
	// より根本のクラスが優先される
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID2, ID::DepthStencil, PSEQ_DSFORMAT)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID2, ID::Stencil, PSEQ_STENCILFORMAT)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID2, ID::Depth, PSEQ_DEPTHFORMAT)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID2, ID::Type, SEQ_TYPE)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID2, ID::Internal_Read, SEQ_INTERNALREAD)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID2, ID::Internal_Render, SEQ_INTERNALRENDER)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID2, ID::Internal_Compressed, SEQ_INTERNALCOMPRESSED)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID2, ID::Internal_Sized, SEQ_INTERNALSIZED)
	BOOST_PP_SEQ_FOR_EACH(ADD_FMTID2, ID::Internal, SEQ_INTERNAL)
}
#undef ADD_FMTID1
#undef ADD_FMTID2
