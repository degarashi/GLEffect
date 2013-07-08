#pragma once
#define BOOST_PP_VARIADICS 1
#include <boost/preprocessor.hpp>
#include <initializer_list>

#define SEQ_VECTOR (x)(y)(z)(w)
#define DEF_VEC(n) \
struct vec##n { \
union { \
	struct { \
		float BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_SUBSEQ(SEQ_VECTOR, 0, n)); \
	}; \
	float value[n]; \
}; \
vec##n() = default; \
vec##n(std::initializer_list<float> il) { assert(il.size() == n); \
	auto* mp = value; \
	for(auto itr=il.begin() ; itr!=il.end() ; itr++) \
		*mp++ = *itr; \
} \
bool operator == (const vec##n& v) const { \
	for(int i=0 ; i<countof(value) ; i++) \
		if(value[i] != v.value[i]) \
			return false; \
	return true; \
} \
};

DEF_VEC(4)
DEF_VEC(3)

struct Mat33 {
	float m[3*3];
	Mat33() = default;
	Mat33(std::initializer_list<float> il) {
		assert(il.size() == countof(m));
		auto* mp = m;
		for(auto itr=il.begin() ; itr!=il.end() ; itr++)
			*mp++ = *itr;
	}

	bool operator == (const Mat33& tm) const {
		for(int i=0 ; i<countof(m) ; i++) {
			if(m[i] != tm.m[i])
				return false;
		}
		return true;
	}
};
struct Mat23 {
	float m[3*2];

	Mat23() = default;
	Mat23(std::initializer_list<float> il) {
		assert(il.size() == countof(m));
		auto* mp = m;
		for(auto itr=il.begin() ; itr!=il.end() ; itr++)
			*mp++ = *itr;
	}

	bool operator == (const Mat23& tm) const {
		for(int i=0 ; i<countof(m) ; i++) {
			if(m[i] != tm.m[i])
				return false;
		}
		return true;
	}
	Mat33 toMat33() const {
		return Mat33{m[0],m[1],0,
						m[2],m[3],0,
						m[4],m[5],1};
	}
};
