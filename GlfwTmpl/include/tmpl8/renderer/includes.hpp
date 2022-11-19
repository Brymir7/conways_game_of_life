#pragma once

// Include GLAD before GLFW
#if defined(_WIN32)

// Prevent inclusion of windows.h
#define TMPL8_OLD_WIN32 WIN32
#undef _WIN32
#include <glad/glad.h>
#define _WIN32 TMPL8_OLD_WIN32
#undef TMPL8_OLD_WIN32

#else
#include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>

#if defined(NDEBUG)
#define TMPL8_RENDERER_CHECK_ERRORS() static_cast<void>(0)
#else
#define TMPL8_RENDERER_CHECK_ERRORS() ::tmpl8::renderer::_check_opengl_errors(__FILE__, __LINE__)
#endif

namespace tmpl8
{
	namespace renderer
	{
#if !defined(NDEBUG)
		void _check_opengl_errors(const char* file, int line);
#endif
	}
}
