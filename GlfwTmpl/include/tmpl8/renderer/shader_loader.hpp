#pragma once

#include <tmpl8/renderer/includes.hpp>

namespace tmpl8
{
	namespace renderer
	{
		GLuint create_shader_program(const char* vertex_shader_file, const char* fragment_shader_file);
	}
}
