#include "glx_parse.hpp"

#define DEF_ERR(r, msg) r.name(msg); qi::on_error<qi::fail>(r, err);
GR_Glx::GR_Glx(): GR_Glx::base_type(rlGLX, "OpenGL_effect_parser") {
	using boost::phoenix::at_c;
	using boost::phoenix::push_back;
	using boost::phoenix::val;
	using boost::phoenix::construct;
	using boost::phoenix::insert;
	using boost::phoenix::construct;

	rlString %= lit('"') > +(standard::char_ - '"') > '"';
	rlNameToken %= qi::lexeme[+(standard::alnum | standard::char_('_'))];
	rlBracket %= lit(_r1) > *(standard::char_ - lit(_r1) - lit(_r2)) > -(rlBracket(_r1,_r2)) > lit(_r2);

	rlAttrEnt = -(GLPrecision[at_c<0>(_val)=_1]) >> GLType[at_c<1>(_val)=_1] >> rlNameToken[at_c<2>(_val)=_1] >> ':' > GLSem[at_c<3>(_val)=_1] > ';';
	rlVaryEnt %= -(GLPrecision) >> GLType >> rlNameToken >> ';';

	// (prec) valueType valueName<sizeSem> : defaultStr
	// valueName<sizeSem> = defaultStr
	rlUnifEnt = -(GLPrecision)[at_c<0>(_val)=_1] > GLType[at_c<1>(_val)=_1] > rlNameToken[at_c<2>(_val)=_1] >
				-('<' > rlNameToken[at_c<3>(_val)=_1] > '>') >
				-(lit('=') > (rlVec | rlNameToken)[at_c<4>(_val)=_1]) > ';';
	rlVec %= '[' > +qi::float_ > ']';
	rlMacroEnt %= rlNameToken > -('=' > rlNameToken) > ';';
	rlConstEnt = -(GLPrecision[at_c<0>(_val)=_1]) >> GLType[at_c<1>(_val)=_1] >> rlNameToken[at_c<2>(_val)=_1] >>
		lit('=') > (rlVec[at_c<3>(_val)=_1] | qi::float_[push_back(at_c<3>(_val),_1)]) > ';';
	rlBoolSet %= qi::no_case[GLBoolsetting] > '=' > qi::no_case[qi::bool_] > ';';
	rlValueSet %= qi::no_case[GLSetting] > '=' >
			qi::repeat(1,4)[qi::no_case[GLColormask] | qi::uint_ | qi::float_ | qi::bool_] > ';';
	rlBlockUse %= qi::no_case[GLBlocktype] > '=' > (rlNameToken % ',') > ';';
	rlShSet = qi::no_case[GLShadertype][at_c<0>(_val)=_1] > '=' > rlNameToken[at_c<1>(_val)=_1] > lit('(') >
		(-(rlVec|qi::bool_|qi::float_)[push_back(at_c<2>(_val), _1)] > *(lit(',') > (rlVec|qi::bool_|qi::float_)[push_back(at_c<2>(_val), _1)])) >
		lit(");");

	rlAttrBlock %= lit("attribute") > rlNameToken > -(':' > (rlNameToken % ',')) >
						'{' > (*rlAttrEnt) > '}';
	rlVaryBlock %= lit("varying") > rlNameToken > -(':' > (rlNameToken % ',')) >
						'{' > (*rlVaryEnt) > '}';
	rlUnifBlock %= lit("uniform") > rlNameToken > -(':' > (rlNameToken % ',')) >
						'{' > (*rlUnifEnt) > '}';
	rlConstBlock %= lit("const") > rlNameToken > -(':' > (rlNameToken % ',')) >
						'{' > (*rlConstEnt) > '}';
	rlArg %= GLType > rlNameToken;
	rlShBlock = qi::no_case[GLShadertype][at_c<0>(_val)=_1] > rlNameToken[at_c<1>(_val)=_1] > '(' >
					-(rlArg[push_back(at_c<2>(_val),_1)] > *(',' > rlArg[push_back(at_c<2>(_val),_1)])) > ')' >
					lit('{') > (qi::lexeme[qi::as_string[*qi::lexeme[standard::char_ - '}']]])[at_c<3>(_val)=_1] > '}';
	rlMacroBlock %= qi::no_case[lit("macro")] > '{' > *rlMacroEnt > '}';
	rlPassBlock = lit("pass") > rlNameToken[at_c<0>(_val)=_1] > '{' >
			*(rlBlockUse[push_back(at_c<1>(_val),_1)] | rlBoolSet[push_back(at_c<2>(_val),_1)] |
			rlMacroBlock[at_c<3>(_val)=_1] | rlShSet[push_back(at_c<4>(_val), _1)] |
			rlValueSet[push_back(at_c<6>(_val),_1)]) > '}';
	rlTechBlock = lit("technique") > rlNameToken[at_c<0>(_val)=_1] > '{' >
			*(rlPassBlock[push_back(at_c<5>(_val),_1)] | rlBlockUse[push_back(at_c<1>(_val),_1)] | rlBoolSet[push_back(at_c<2>(_val),_1)] |
			rlMacroBlock[at_c<3>(_val)=_1] | rlShSet[push_back(at_c<4>(_val), _1)] |
			rlValueSet[push_back(at_c<6>(_val),_1)]) > '}';
	rlGLX = *(rlComment | (rlAttrBlock[insert(at_c<0>(_val), construct<std::pair<std::string,AttrStruct>>(at_c<0>(_1), _1))] |
				rlConstBlock[insert(at_c<1>(_val), construct<std::pair<std::string,ConstStruct>>(at_c<0>(_1), _1))] |
				rlShBlock[insert(at_c<2>(_val), construct<std::pair<std::string,ShStruct>>(at_c<1>(_1), _1))] |
				rlTechBlock[push_back(at_c<3>(_val), _1)] |
				rlUnifBlock[insert(at_c<4>(_val), construct<std::pair<std::string,UnifStruct>>(at_c<0>(_1), _1))] |
				rlVaryBlock[insert(at_c<5>(_val), construct<std::pair<std::string,VaryStruct>>(at_c<0>(_1), _1))]));
	rlCommentS = lit("//") > qi::lexeme[*(standard::char_ - qi::eol - qi::eoi) > (qi::eol | qi::eoi)];
	rlCommentL = lit("/*") > qi::lexeme[*(standard::char_ - qi::lit("*/")) > qi::lit("*/")];
	rlComment = rlCommentS | rlCommentL;

	auto err = (std::cout << val("Error! Expectiong ")
		<< _4 << val(" here: \"")
		<< construct<std::string>(_3, _2)
		<< val("\"") << std::endl);

	DEF_ERR(rlBlockUse, "block-use_parser")
	DEF_ERR(rlConstEnt, "const_entry_parser")
	DEF_ERR(rlConstBlock, "const_block_parser")
	DEF_ERR(rlComment, "comment_parser")
	DEF_ERR(rlCommentL, "long_comment_parser")
	DEF_ERR(rlCommentS, "short_comment_parser")
	DEF_ERR(rlString, "quoted_string_parser")
	DEF_ERR(rlNameToken, "name_token_parser")
	DEF_ERR(rlBracket, "bracket_parser")
	DEF_ERR(rlAttrEnt, "attribute_entry_parser")
	DEF_ERR(rlVaryEnt, "varying_entry_parser")
	DEF_ERR(rlUnifEnt, "uniform_entry_parser")
	DEF_ERR(rlMacroEnt, "macro_entry_parser")
	DEF_ERR(rlVec, "vector_value_parser")
	DEF_ERR(rlBoolSet, "boolean_setting_parser")
	DEF_ERR(rlValueSet, "value_setting_parser")
	DEF_ERR(rlShSet, "shader_setting_parser")
	DEF_ERR(rlShBlock, "shader_definition_parser")
	DEF_ERR(rlAttrBlock, "attribute_block_parser")
	DEF_ERR(rlVaryBlock, "varying_block_parser")
	DEF_ERR(rlUnifBlock, "uniform_block_parser")
	DEF_ERR(rlMacroBlock, "macro_block_parser")
	DEF_ERR(rlPassBlock, "pass_block_parser")
	DEF_ERR(rlTechBlock, "technique_block_parser")
	DEF_ERR(rlGLX, "OpenGL_effect_parser")
}
