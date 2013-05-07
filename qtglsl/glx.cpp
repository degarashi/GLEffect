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
VDecl::VDecl(std::initializer_list<VDInfo> il) {
	// StreamID毎に集計
	std::vector<VDInfo> tmp[VData::MAX_STREAM];
	for(auto& v : il)
		tmp[v.streamID].push_back(v);

	// 頂点定義のダブり確認
	for(auto& t : tmp) {
		// オフセットでソート
		std::sort(t.begin(), t.end(), [](const VDInfo& v0, const VDInfo& v1) { return v0.offset < v1.offset; });

		uint ofs = 0;
		for(auto& t2 : t) {
			if(ofs > t2.offset)
				throw GLE_Error("invalid vertex offset");
			ofs += GLBuffer::GetUnitSize(t2.elemFlag) * t2.elemSize;
		}
	}

	_func.resize(il.size());
	int cur = 0;
	for(int i=0 ; i<countof(tmp) ; i++) {
		_nEnt[i] = cur;
		for(auto& t2 : tmp[i]) {
			_func[cur] = [&t2](GLuint stride, const VData::AttrA& attr) {
				auto attrID = attr[t2.semID];
				if(attrID < 0)
					return;

				glVertexAttribPointer(attrID, t2.elemSize, t2.elemFlag, t2.bNormalize, stride, (const GLvoid*)t2.offset);
				GLCheck()
			};
		}

		++cur;
	}
	_nEnt[VData::MAX_STREAM] = _nEnt[VData::MAX_STREAM-1];
}
void VDecl::apply(const VData& vdata) const {
	for(int i=0 ; i<VData::MAX_STREAM ; i++) {
		auto& sp = vdata.spBuff[i];
		glBindBuffer(GL_ARRAY_BUFFER, sp->getBuffID());
		GLCheck()

		GLuint stride = sp->getStride();
		for(int j=_nEnt[i] ; j<_nEnt[i+1] ; j++)
			_func[j](stride, vdata.attrID);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// ----------------- TPStructR -----------------
TPStructR::TPStructR() {}
TPStructR::TPStructR(TPStructR&& tp) {
	swap(tp);
}
void TPStructR::swap(TPStructR& t) noexcept {
	boost::swap(_prog, t._prog);
	boost::swap(_vAttrID, t._vAttrID);
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
	std::copy(to._vAttrID, to._vAttrID+countof(_vAttrID), ret._vAttrID);
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
	_ost << GLType_::cs_typeStr[arg->type] << ' ' << arg->name;
	++_cursor;
}
void ArgChecker::operator()(const std::vector<float>& v) {
	int typ = _arg[_cursor]->type;
	_checkAndSet(VECTOR);
	_ost << '=' << GLType_::cs_typeStr[typ] << '(';
	int nV = v.size();
	for(int i=0 ; i<nV-1 ; i++)
		_ost << v[i] << ',';
	_ost << v.back() << ");" << std::endl;
}
void ArgChecker::operator()(float v) {
	_checkAndSet(SCALAR);
	_ost << v << ';' << std::endl;
}
void ArgChecker::operator()(bool b) {
	_checkAndSet(BOOLEAN);
	_ost << b << ';' << std::endl;
}
void ArgChecker::finalizeCheck() {
	if(_cursor >=countof(_target))
		return;
	if(_target[_cursor] != NONE)
		throw GLE_InvalidArgument(_shName, "(missing arguments)");
}

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
void GLEffect::setVDecl(const SPVDecl& decl) {
	_spVDecl = decl;
}

namespace {
	class TPSDupl {
		using TPList = std::vector<const TPStruct*>;
		const GLXStruct &_glx;
		const TPStruct &_tTech, &_tPass;
		//! [Pass][Tech][Tech(Base0)][Tech(Base1)]...
		TPList _tpList;

		void _getTPStruct(TPList& dst, const TPStruct* tp) const {
			dst.push_back(tp);
			auto& der = tp->derive;
			for(auto& name : der) {
				auto itr = std::find_if(_glx.tpL.begin(), _glx.tpL.end(), [&name](const boost::recursive_wrapper<TPStruct>& t){return t.get().name == name;});
				_getTPStruct(dst, &(*itr));
			}
		}
		public:
			TPSDupl(const GLXStruct& gs, int tech, int pass): _glx(gs), _tTech(gs.tpL.at(tech)), _tPass(_tTech.tpL.at(pass).get()) {
				_tpList.push_back(&_tPass);
				// 継承関係をリストアップ
				_getTPStruct(_tpList, &_tTech);
			}

			template <class ST>
			void _extractBlocks(std::vector<const ST*>& dst, const ST* attr, const NameMap<ST> (GLXStruct::*mfunc)) const {
				for(auto itr=attr->derive.rbegin() ; itr!=attr->derive.rend() ; itr++) {
					auto* der = &(_glx.*mfunc).at(*itr);
					_extractBlocks(dst, der, mfunc);
				}
				if(std::find(dst.begin(), dst.end(), attr) == dst.end())
					dst.push_back(attr);
			}

			template <class ST, class ENT>
			std::vector<const ENT*> exportEntries(int blockID, const std::map<std::string,ST> (GLXStruct::*mfunc)) const {
				// 使用されるAttributeブロックを収集
				std::vector<const ST*> tmp, tmp2;
				// 配列末尾から処理をする
				for(auto itr=_tpList.rbegin() ; itr!=_tpList.rend() ; itr++) {
					const TPStruct* tp = (*itr);
					// ブロックは順方向で操作 = 後に書いたほうが優先
					for(auto& blk : tp->blkL) {
						if(blk.type == blockID) {
							if(!blk.bAdd)
								tmp.clear();
							for(auto& name : blk.name)
								tmp.push_back(&(_glx.*mfunc).at(name));
						}
					}
				}
				// ブロック継承展開
				for(auto& p : tmp) {
					// 既に同じブロックが登録されていたら何もしない(エントリの重複を省く)
					_extractBlocks(tmp2, p, mfunc);
				}
				// エントリ抽出: 同じ名前のエントリがあればエラー = 異なるエントリに同じ変数が存在している
				std::vector<const ENT*> ret;
				for(auto& p : tmp2) {
					for(auto& e : p->entry) {
						if(std::find_if(ret.begin(), ret.end(), [&e](const ENT* tmp){return e.name==tmp->name;}) != ret.end())
							throw GLE_LogicalError((boost::format("duplication of entry \"%1%\"") % e.name).str());
						ret.push_back(&e);
					}
				}
				return ret;
			}
			using MacroPair = std::pair<std::string, std::string>;
			using MacroMap = std::map<std::string, std::string>;
			MacroMap exportMacro() const {
				MacroMap mm;
				for(auto itr=_tpList.rbegin() ; itr!=_tpList.rend() ; itr++) {
					for(auto& mc : (*itr)->mcL) {
						MacroPair mp(mc.fromStr, mc.toStr ? (*mc.toStr) : std::string());
						mm.insert(std::move(mp));
					}
				}
				return std::move(mm);
			}
			SettingList exportSetting() const {
				std::vector<ValueSettingR> vsL;
				std::vector<BoolSettingR> bsL;
				for(auto itr=_tpList.rbegin() ; itr!=_tpList.rend() ; itr++) {
					const TPStruct* tp = (*itr);
					// フラグ設定エントリ
					for(auto& bs : tp->bsL) {
						// 実行時形式に変換してからリストに追加
						BoolSettingR bsr(bs);
						auto itr=std::find(bsL.begin(), bsL.end(), bsr);
						if(itr == bsL.end()) {
							// 新規に追加
							bsL.push_back(bsr);
						} else {
							// 既存の項目を上書き
							*itr = bsr;
						}
					}

					// 値設定エントリ
					for(auto& vs : tp->vsL) {
						ValueSettingR vsr(vs);
						auto itr=std::find(vsL.begin(), vsL.end(), vsr);
						if(itr == vsL.end())
							vsL.push_back(vsr);
						else
							*itr = vsr;
					}
				}
				SettingList ret;
				for(auto& b : bsL)
					ret.push_back(b);
				for(auto& v : vsL)
					ret.push_back(v);
				return std::move(ret);
			}
	};

	template <class DST, class SRC>
	void OutputS(DST& dst, const SRC& src) {
		for(auto& p : src) {
			p->output(dst);
			dst << std::endl;
		}
	}
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
	TPSDupl dupl(gs, tech, pass);
	SPShader shP[ShType::NUM_SHTYPE];
	std::vector<const AttrEntry*> attL;
	for(int i=0 ; i<countof(selectSh) ; i++) {
		auto* shp = selectSh[i];
		if(!shp)
			continue;

		{
			auto macro = dupl.exportMacro();
			for(auto& p : macro)
				ss << "#define " << p.first << ' ' << p.second << std::endl;
		}
		if(i==ShType::VERTEX) {
			// attL: 後で頂点関連付けにも使う
			attL = dupl.exportEntries<AttrStruct,AttrEntry>(GLBlocktype_::attributeT, &GLXStruct::atM);
			for(auto& p : attL) {
				p->output(ss);
				ss << std::endl;
			}
		}
		{	auto varL = dupl.exportEntries<VaryStruct,VaryEntry>(GLBlocktype_::varyingT, &GLXStruct::varM);
			OutputS(ss, varL); }
		{	auto csL = dupl.exportEntries<ConstStruct,ConstEntry>(GLBlocktype_::constT, &GLXStruct::csM);
			OutputS(ss, csL); }
		{	auto unifL = dupl.exportEntries<UnifStruct,UnifEntry>(GLBlocktype_::uniformT, &GLXStruct::uniM);
			OutputS(ss, unifL); }

		const ShStruct& s = gs.shM.at(shp->shName);
		// シェーダー引数の型チェック
		// ユーザー引数はグローバル変数として用意
		ArgChecker acheck(ss, shp->shName, s.args);
		for(auto& a : shp->args)
			boost::apply_visitor(acheck, a);
		acheck.finalizeCheck();

		// main()関数に書き換え
		ss << "void main() {" << s.info << '}' << std::endl;

		std::cout << ss.str();
		std::cout.flush();
		shP[i].reset(new GLShader(c_glShFlag[i], ss.str()));

		ss.str("");
		ss.clear();
	}
	// シェーダーのリンク処理
	_prog.reset(new GLProgram(shP[0], shP[1], shP[2]));
	// 頂点AttribIDを無効な値で初期化
	for(auto& v : _vAttrID)
		v = -2;	// 初期値=-2, クエリの無効値=-1
	for(auto& p : attL) {
		// 頂点セマンティクス対応リストを生成
		// セマンティクスの重複はエラー
		auto& atID = _vAttrID[p->sem];
		if(atID != -2)
			throw GLE_LogicalError((boost::format("duplication of vertex semantics \"%1% : %2%\"") % p->name % GLSem_::cs_typeStr[p->sem]).str());
		atID = glGetAttribLocation(_prog->getProgramID(), p->name.c_str());
		GLCheck()
		// -1の場合は警告を出す(もしかしたらシェーダー内で使ってないだけかもしれない)
	}

	// OpenGLステート設定リストを形成
	SettingList sl = dupl.exportSetting();
	_setting.swap(sl);

	applySetting();
}

void TPStructR::applySetting() const {
	struct Visitor : boost::static_visitor<> {
		void operator()(const BoolSettingR& bs) const {
			bs.action();
		}
		void operator()(const ValueSettingR& vs) const {
			vs.action();
		}
		void operator()(const DefVal& v) const {}
	};
	for(auto& st : _setting)
		boost::apply_visitor(Visitor(), st);
}

void TPStructR::setVertex(const VDecl &vdecl, const SPBuffer (&stream)[VData::MAX_STREAM]) const {
	_prog->use();
	vdecl.apply(VData(stream, _vAttrID));
}
