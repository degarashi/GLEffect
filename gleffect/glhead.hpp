#pragma once

#include <GL/gl.h>
#if !defined(_WIN32)
	#include <GL/glx.h>
#endif
#undef Convex
#include "glext.h"
#if !defined(_WIN32)
	#include "glxext.h"
#endif
#define GLDEFINE(name,type)		extern type name;
#include "glfunc.inc"
#undef GLDEFINE

extern void LoadGLFunc();
extern bool IsGLFuncLoaded();
extern void SetSwapInterval(int n);
