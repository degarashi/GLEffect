#pragma once
#define BOOST_PP_VARIADICS 1
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

	struct IPolicy {
		virtual void onOutput(const std::string& msg) const = 0;
		virtual bool onError(const std::string& msg) const = 0;
	};

	//! ダイアログを出し、例外を投げる
	static struct _Policy_Critical : IPolicy {
		//! エラーの出力方法
		void onOutput(const std::string& msg) const override {
			throw std::runtime_error(msg);
		}
		//! エラー時の表示
		/*! \return Ignoreフラグ */
		bool onError(const std::string& msg) const override {
			int res = QMessageBox::warning(nullptr, "assertion failed!", msg.c_str(), QMessageBox::Abort|QMessageBox::Retry|QMessageBox::Ignore);
			if(res == QMessageBox::Abort) {
				onOutput(msg);
				__asm__("int $0x03;");
				return false;
			} else
				return res == QMessageBox::Ignore;
		}
	} Policy_Critical;

	//! 進行を止めずに警告だけ出す
	static struct _Policy_Warning : IPolicy {
		void onOutput(const std::string& msg) const override {
			std::cout.write(msg.c_str(), msg.length());
			std::cout << std::endl;
		}
		bool onError(const std::string& msg) const override {
			onOutput(msg);
			return false;
		}
	} Policy_Warning;

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
		ss << Tmp(fmt, ts...).str() << endl;
		return ss.str();
	}

	//! アサート無視チェック後、エラー出力
	template <class... Ts>
	static bool checkAssert(IPolicy* policy, CStr cond, CStr file, CStr func, int line, const std::string& info, const Ts&... ts) {
		std::string errStr(makeErrString(file, func, line, cond, info, ts...));
		return policy->onError(errStr);
	}
};
#define FUNCTIONNAME __func__
#ifdef DEBUG
	#define AAssert(cond) AssertArg(cond, "")
	#define AAssertArg(cond, ...) { static bool bIgnored = false; \
	if(!bIgnored && !(cond)) { bIgnored = DGAssert::checkAssert(&DGAssert::Policy_Critical, #cond, __FILE__, FUNCTIONNAME, __LINE__, __VA_ARGS__); } }
	#define AWarn(cond) WarnArg(cond, "")
	#define AWarnArg(cond, ...) { if(!(cond)) { DGAssert::checkAssert(&DGAssert::Policy_Warning, #cond, __FILE__, FUNCTIONNAME, __LINE__, __VA_ARGS__); }}
#else
	#define AAssert(cond)
	#define AAssertArg(cond, ...)
	#define AWarn(cond)
	#define AWarnArg(cond, ...)
#endif

#define Assert(cond) AssertArg(cond, "")
#define AssertArg(cond, ...) { static bool bIgnored = false; \
if(!bIgnored && !(cond)) { bIgnored = DGAssert::checkAssert(&DGAssert::Policy_Critical, #cond, __FILE__, FUNCTIONNAME, __LINE__, __VA_ARGS__); } }
#define Warn(cond) WarnArg(cond, "")
#define WarnArg(cond, ...) { if(!(cond)) { DGAssert::checkAssert(&DGAssert::Policy_Warning, #cond, __FILE__, FUNCTIONNAME, __LINE__, __VA_ARGS__); }}
