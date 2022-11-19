#include <tmpl8/renderer/includes.hpp>
#include <iostream>
#include <sstream>

void tmpl8::renderer::_check_opengl_errors(const char* file, int line)
{
	const char* msg;
	switch (glGetError())
	{
	case GL_NO_ERROR: return;
	case GL_INVALID_ENUM: msg = "Invalid enum."; break;
	case GL_INVALID_VALUE: msg = "Invalid value."; break;
	case GL_INVALID_OPERATION: msg = "Invalid operation."; break;
	case GL_INVALID_FRAMEBUFFER_OPERATION: msg = "Invalid framebuffer operation."; break;
	case GL_OUT_OF_MEMORY: msg = "Out of memory."; break;
	// case GL_STACK_UNDERFLOW: msg = "Stack underflow."; break;
	// case GL_STACK_OVERFLOW: msg = "Stack overflow."; break;
	default: msg = "Unknown error."; break;
	}

	std::stringstream sstr;
	sstr << "OpenGL error occured, tested at: " << file << ":" << line << '\n' << "Message: " << msg << '\n';

	std::cerr << sstr.str();
#if defined(_MSC_VER)
	__debugbreak();
#endif
	std::terminate();
}
