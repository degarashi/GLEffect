#pragma once

#include <GL/gl.h>
#include <GL/glx.h>
#undef Convex
#include "glext.h"
#include "glxext.h"
#define GLDEFINE(name,type)		extern type name;
#include "glfunc.inc"
#undef GLDEFINE

extern void LoadXGLFunc();
