#pragma once
#include <QMessageBox>
#include <iostream>
#include <sstream>
#include <boost/format.hpp>

struct DGAssert {
	template <class FMT>
	static FMT& Tmp(FMT& fmt) { return fmt; }
	template <class FMT, class T, class... Ts>
	static FMT& Tmp(FMT& fmt, const T& t, const Ts&... ts) {
		fmt % t;
		return Tmp(fmt, ts...);
	}

	static void output_Throw(const std::string& msg) {
		throw std::runtime_error(msg);
	}
	static void output_Print(const std::string& msg) {
		std::cout << msg;
	}

	using CStr = const char*;
	template <class... Ts>
	static std::string makeErrString(CStr file, CStr func, int line, const std::string& cause, const std::string& info, const Ts&... ts) {
		using std::endl;
		std::stringstream ss;
		ss << cause << "at" << endl
			<< "file=" << file << endl
			<< "func=" << func << endl
			<< "line=" << line << endl;
		boost::format fmt(info);
		ss << Tmp(fmt, ts...).str();
		return ss.str();
	}

	//! アサート無視チェック後、エラー出力
	template <class OUTF, class... Ts>
	static bool checkAssert(OUTF outf, CStr cond, CStr file, CStr func, int line, const std::string& info, const Ts&... ts) {
		int res = QMessageBox::warning(nullptr, "assertion failed!", cond, QMessageBox::Abort|QMessageBox::Retry|QMessageBox::Ignore);
		if(res == QMessageBox::Abort) {
			std::stringstream ss;
			ss << "assertion failed!" << std::endl
				  << cond << std::endl;
			outf(makeErrString(file, func, line, ss.str(), info, ts...));
			return false;
		} else
			return res == QMessageBox::Ignore;
	}
};
#define FUNCTIONNAME __func__
#ifdef DEBUG
	#define Assert(cond) AssertArg(cond, "")
	#define AssertArg(cond, ...) { static bool bIgnored = false; \
	if(!bIgnored && !(cond)) { bIgnored = DGAssert::checkAssert(DGAssert::output_Throw, #cond, __FILE__, FUNCTIONNAME, __LINE__, __VA_ARGS__); } }
	#define Warn(cond) WarnArg(cond, "")
	#define WarnArg(cond, ...) DGAssert::output_Print(DGAssert::makeErrString(__FILE__, FUNCTIONNAME, __LINE__, #cond, __VA_ARGS__));
#else
	#define Assert(cond)
	#define AssertArg(cond, ...)
	#define Warn(cond)
	#define WarnArg(cond, ...)
#endif
