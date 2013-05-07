#pragma once
#define BOOST_PP_VARIADICS 1
#include <boost/preprocessor.hpp>

#define SEQ_VECTOR (x)(y)(z)(w)
#define DEF_VEC(n) \
struct vec##n { \
union { \
	struct { \
		float BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_SUBSEQ(SEQ_VECTOR, 0, n)); \
	}; \
	float value[n]; \
}; \
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
