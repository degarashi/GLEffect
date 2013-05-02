#include "glx.hpp"
#include <boost/format.hpp>
#include <fstream>

const GLType_ GLType;
const GLInout_ GLInout;
const GLSem_ GLSem;
const GLPrecision_ GLPrecision;
const GLBoolsetting_ GLBoolsetting;
const GLSetting_ GLSetting;
const GLStencilop_ GLStencilop;
const GLFunc_ GLFunc;
const GLEq_ GLEq;
const GLBlend_ GLBlend;
const GLFace_ GLFace;
const GLFacedir_ GLFacedir;
const GLColormask_ GLColormask;
const GLShadertype_ GLShadertype;
const GLBlocktype_ GLBlocktype;

const char* GLType_::cs_typeStr[] = {
	BOOST_PP_SEQ_FOR_EACH(PPFUNC_STR, EMPTY, SEQ_GLTYPE)
};
const char* GLSem_::cs_typeStr[] = {
	BOOST_PP_SEQ_FOR_EACH(PPFUNC_STR, EMPTY, SEQ_VSEM)
};
const VSFunc ValueSettingR::cs_func[] = {
	BOOST_PP_SEQ_FOR_EACH(PPFUNC_GLSET_FUNC, EMPTY, SEQ_GLSETTING)
};
const char* GLPrecision_::cs_typeStr[] = {
	BOOST_PP_SEQ_FOR_EACH(PPFUNC_STR, EMPTY, SEQ_PRECISION)
};
const char* GLSetting_::cs_typeStr[] = {
	BOOST_PP_SEQ_FOR_EACH(PPFUNC_STR, EMPTY, SEQ_GLSETTING)
};
const char* GLBlocktype_::cs_typeStr[] = {
	BOOST_PP_SEQ_FOR_EACH(PPFUNC_STR, EMPTY, SEQ_BLOCK)
};

// -------------- ValueSettingR --------------
void ValueSettingR::StencilFuncFront(int func, int ref, int mask) {
	glStencilFuncSeparate(GL_FRONT, func, ref, mask);
}
void ValueSettingR::StencilFuncBack(int func, int ref, int mask) {
	glStencilFuncSeparate(GL_BACK, func, ref, mask);
}
void ValueSettingR::StencilOpFront(int sfail, int dpfail, int dppass) {
	glStencilOpSeparate(GL_FRONT, sfail, dpfail, dppass);
}
void ValueSettingR::StencilOpBack(int sfail, int dpfail, int dppass) {
	glStencilOpSeparate(GL_BACK, sfail, dpfail, dppass);
}
void ValueSettingR::StencilMaskFront(int mask) {
	glStencilMaskSeparate(GL_FRONT, mask);
}
void ValueSettingR::StencilMaskBack(int mask) {
	glStencilMaskSeparate(GL_BACK, mask);
}
ValueSettingR::ValueSettingR(const ValueSetting& s) {
	func = cs_func[s.type];
	int nV = std::min(static_cast<int>(s.value.size()), countof(value));
	for(int i=0 ; i<nV ; i++)
		value[i] = s.value[i];
	for(int i=nV ; i<countof(value) ; i++)
		value[i] = boost::blank();
}
void ValueSettingR::action() const { func(*this); }
bool ValueSettingR::operator == (const ValueSettingR& s) const {
	for(int i=0 ; i<countof(value) ; i++)
		if(!(value[i] == s.value[i]))
			return false;
	return func == s.func;
}

// -------------- BoolSettingR --------------
const VBFunc BoolSettingR::cs_func[] = {
	glEnable, glDisable
};
BoolSettingR::BoolSettingR(const BoolSetting& s) {
	func = cs_func[(s.value) ? 0 : 1];
	flag = s.type;
}
void BoolSettingR::action() const { func(flag); }
bool BoolSettingR::operator == (const BoolSettingR& s) const {
	return flag==s.flag && func==s.func;
}

GLEffect::EC_FileNotFound::EC_FileNotFound(const std::string& fPath):
	EC_Base((boost::format("file path: \"%1%\" was not found.") % fPath).str()) {}
// ----------------- VDecl -----------------
VDecl::VDecl() {}
VDecl::VDecl(std::initializer_list<VDArray> il) {
	_ar.resize(il.size());
	std::copy(il.begin(), il.end(), _ar.begin());
}
void VDecl::apply(const GLuint* ids, int n) const {}
// ----------------- TPStructR -----------------
TPStructR::TPStructR() {}
TPStructR::TPStructR(TPStructR&& tp) {
	swap(tp);
}
void TPStructR::swap(TPStructR& t) noexcept {
	boost::swap(_prog, t._prog);
	boost::swap(_vsem, t._vsem);
	boost::swap(_setting, t._setting);
}
bool TPStructR::findSetting(const Setting& s) const {
	auto itr = std::find(_setting.begin(), _setting.end(), s);
	return itr!=_setting.end() && (*itr)==s;
}
TPStructR TPStructR::calcDiff(const TPStructR& from, const TPStructR& to) {
	// toと同じ設定がfrom側にあればスキップ
	// fromに無かったり、異なっていればエントリに加える
	TPStructR ret;
	for(auto& s : to._setting) {
		if(!from.findSetting(s)) {
			ret._setting.push_back(s);
		}
	}
	std::copy(to._vsem, to._vsem+countof(_vsem), ret._vsem);
	return std::move(ret);
}

ShStruct& ShStruct::operator = (const ShStruct& a) {
	this->~ShStruct();
	new(this) ShStruct(a);
	return *this;
}
ShStruct::ShStruct(ShStruct&& a): ShStruct() { swap(a); }
void ShStruct::swap(ShStruct& a) noexcept {
	std::swap(type, a.type);
	std::swap(name, a.name);
	std::swap(args, a.args);
	std::swap(info, a.info);
}
// ----------------- ArgChecker -----------------
ArgChecker::ArgChecker(std::ostream& ost, const std::string& shName, const std::vector<ArgItem>& args):_shName(shName), _ost(ost) {
	int nA = args.size();
	for(int i=0 ; i<nA ; i++) {
		_target[i] = Detect(args[i].type);
		_arg[i] = &args[i];
	}
	for(int i=nA ; i<N_TARGET ; i++) {
		_target[i] = NONE;
		_arg[i] = nullptr;
	}
}
ArgChecker::TARGET ArgChecker::Detect(int type) {
	if(type <= GLType_::TYPE::boolT)
		return BOOLEAN;
	if(type <= GLType_::TYPE::floatT)
		return SCALAR;
	if(type <= GLType_::TYPE::ivec4T)
		return VECTOR;
	return NONE;
}
void ArgChecker::_checkAndSet(TARGET tgt) {
	auto t = _target[_cursor];
	auto* arg = _arg[_cursor];
	if(t==NONE)
		throw GLE_InvalidArgument(_shName, "(none)");
	if(t!=tgt)
		throw GLE_InvalidArgument(_shName, arg->name);
	_ost << GLType_::cs_typeStr[arg->type] << ' ' << arg->name << ';' << std::endl;
	++_cursor;
}
void ArgChecker::operator()(const std::vector<float>& v) { _checkAndSet(VECTOR); }
void ArgChecker::operator()(float v) { _checkAndSet(SCALAR); }
void ArgChecker::operator()(bool b) { _checkAndSet(BOOLEAN); }

// ----------------- GLEffect -----------------
void GLEffect::readGLX(const std::string& fPath) {
	std::ifstream ifs(fPath);
	if(!ifs.is_open())
		throw EC_FileNotFound(fPath);

	GR_Glx glx;
	std::istreambuf_iterator<char> bItrB(ifs), bItrE;
	std::string str(bItrB, bItrE);
	auto itr = str.cbegin();
	GLXStruct result;
	bool bS = boost::spirit::qi::phrase_parse(itr, str.cend(), glx, standard::space, result);
	if(bS)
		std::cout << "------- analysis succeeded! -------" << std::endl;
	else
		std::cout << "------- analysis failed! -------" << std::endl;
	if(itr != str.cend()) {
		std::cout << "<but not reached to end>" << std::endl
			<< "remains: " << std::endl << std::string(itr, str.cend()) << std::endl;
	}

	// テスト表示
	result.output(std::cout);

	// Tech/Passを順に実行形式へ変換
	int nI = result.tpL.size();
	for(int techID=0 ; techID<nI ; techID++) {
		// Pass毎に処理
		int nJ = result.tpL[techID].tpL.size();
		for(int passID=0 ; passID<nJ ; passID++)
			_techMap.insert(std::make_pair(GL16ID(techID, passID), TPStructR(result, techID, passID)));
	}
}

namespace {
	struct TPSDupl {
		const GLXStruct &glx;
		const TPStruct &tTech, &tPass;
		TPSDupl(const GLXStruct& gs, const TPStruct& tech, const TPStruct& pass): glx(gs), tTech(tech), tPass(pass) {}

		template <class T>
		void output(std::ostream& os, const T& src, const std::string& name) const {
			const auto& vs = src.at(name);
			for(auto& d : vs.derive)
				output(os, src, d);
			for(auto& e : vs.entry) {
				e.output(os);
				os << std::endl;
			}
		}

		template <class T>
		const BlockUse* exportBlock(std::ostream& os, const T& src, uint32_t typ) const {
			// 一番最後に宣言されたBlockが有効。+= 演算子は今は対応しない
			auto fn = [typ](const TPStruct& tp) -> const BlockUse* {
				for(auto itr=tp.blkL.rbegin() ; itr!=tp.blkL.rend() ; itr++) {
					if(itr->type == typ)
						return &(*itr);
				}
				return nullptr;
			};

			const BlockUse* blk = fn(tPass);
			if(!blk) {
				// Passに無ければTechから探す
				if(!(blk = fn(tTech)))
					throw GLE_LogicalError(
						(boost::format("no %1% block found in %2% : %3%") % GLBlocktype_::cs_typeStr[typ] % tTech.name % tPass.name).str());
			}

			for(auto& a : blk->name)
				output(os, src, a);
			return blk;
		}
		void exportVarying(std::ostream& os) const {
			exportBlock(os, glx.varM, GLBlocktype_::TYPE::varyingT);
		}
		const BlockUse* exportAttribute(std::ostream& os) const {
			return exportBlock(os, glx.atM, GLBlocktype_::TYPE::attributeT);
		}
		void exportConst(std::ostream& os) const {
			// constブロックは必須ではない
			try {
				exportBlock(os, glx.csM, GLBlocktype_::TYPE::constT);
			} catch(const GLE_LogicalError& e) {}
		}
		void exportUniform(std::ostream& os) const {
			// uniformブロックは必須ではない
			try {
				exportBlock(os, glx.uniM, GLBlocktype_::TYPE::uniformT);
			} catch(const GLE_LogicalError& e) {}
		}
		void exportMacro(std::ostream& os) const {
			// MacroListを合成 (pass + tech)
			for(auto& a : tPass.mcL)
				a.output(os);
			for(auto& a : tTech.mcL) {
				auto itr = std::find_if(tPass.mcL.begin(), tPass.mcL.end(), [&a](const MacroEntry& e) { return a.fromStr==e.fromStr; });
				if(itr == tPass.mcL.end())
					a.output(os);
			}
		}
		void exportVSemantics(std::string (&dst)[static_cast<int>(VSem::NUM_SEMANTIC)], const BlockUse* blk) {

		}
	};
}

TPStructR::TPStructR(const GLXStruct& gs, int tech, int pass) {
	auto& tp = gs.tpL.at(tech);
	auto& tps = tp.tpL[pass].get();
	const ShSetting* selectSh[ShType::NUM_SHTYPE] = {};
	// PassかTechからシェーダー名を取ってくる
	for(auto& a : tp.shL)
		selectSh[a.type] = &a;
	for(auto& a : tps.shL)
		selectSh[a.type] = &a;

	// VertexとPixelシェーダは必須、Geometryは任意
	if(!selectSh[ShType::VERTEX] || !selectSh[ShType::PIXEL])
		throw GLE_LogicalError("no vertex or pixel shader found");

	std::stringstream ss;
	TPSDupl dupl(gs, tp, tps);
	SPShader shP[ShType::NUM_SHTYPE];
	const BlockUse* blk;
	for(int i=0 ; i<countof(selectSh) ; i++) {
		auto* shp = selectSh[i];
		if(!shp)
			continue;

		dupl.exportMacro(ss);
		if(i==ShType::VERTEX)
			blk = dupl.exportAttribute(ss);
		dupl.exportVarying(ss);
		dupl.exportConst(ss);
		dupl.exportUniform(ss);

		const ShStruct& s = gs.shM.at(shp->shName);
		// シェーダー引数の型チェック
		// ユーザー引数はグローバル変数として用意
		ArgChecker acheck(ss, shp->shName, s.args);
		for(auto& a : shp->args)
			boost::apply_visitor(acheck, a);
		// main()関数に書き換え
		ss << "void main() {" << s.info << '}';

		std::cout << ss.str();
		shP[i].reset(new GLShader(c_glShFlag[i], ss.str()));

		ss.str("");
		ss.clear();
	}
}
