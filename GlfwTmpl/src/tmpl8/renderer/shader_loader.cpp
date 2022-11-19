#include <tmpl8/renderer/shader_loader.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <memory>

std::string read_source_file(const char* file_type, const char* file_name)
{
	std::ifstream shader_stream(file_name, std::ios::in);
	if (shader_stream.is_open())
	{
		std::stringstream shader_source;
		shader_source << shader_stream.rdbuf();
		return shader_source.str();
	}

	std::stringstream err_str;
	err_str << "Unable to open " << file_name << ". Type: " << file_type << '\n';

	std::cerr << err_str.str();
#if defined(_MSC_VER)
	__debugbreak();
#endif
	exit(-1);
}

void compile_shader(const char* shader_type, GLuint shader_id, const std::string& shader_source)
{
	// Compile the shader
	const char* sourceData = shader_source.c_str();
	glShaderSource (shader_id, 1, &sourceData, nullptr);
	glCompileShader(shader_id);

	// Check error messages
	GLint result = GL_FALSE;
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result);
	int info_log_length;
	glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &info_log_length);
	if (info_log_length <= 0) return;

	auto error_msg = std::make_unique<char[]>(info_log_length + 1);
	glGetShaderInfoLog(shader_id, info_log_length, nullptr, error_msg.get());

	std::stringstream err_str;
	err_str << "Error compiling shader of type: " << shader_type << ", message:\n" << error_msg.get() << "\n";
	std::cerr << err_str.str();
#if defined(_MSC_VER)
	__debugbreak();
#endif
	exit(-1);
}

GLuint tmpl8::renderer::create_shader_program(const char* vertex_shader_file, const char* fragment_shader_file)
{
	static const char* const vert_shader_type = "Vertex Shader";
	static const char* const frag_shader_type = "Fragment Shader";
	static const char* const geom_shader_type = "Geometry Shader";

	TMPL8_RENDERER_CHECK_ERRORS();

	// Create the shaders
	GLuint vert_shader_id = glCreateShader(GL_VERTEX_SHADER);
	GLuint frag_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

	// Read shader sources
	std::string vert_shader = read_source_file(vert_shader_type, vertex_shader_file);
	std::string frag_shader = read_source_file(frag_shader_type, fragment_shader_file);
	std::string geom_shader;

	// Compile shaders
	compile_shader(vert_shader_type, vert_shader_id, vert_shader);
	compile_shader(frag_shader_type, frag_shader_id, frag_shader);

	// Link the program
	GLuint shader_program_id = glCreateProgram();
	glAttachShader(shader_program_id, vert_shader_id);
	glAttachShader(shader_program_id, frag_shader_id);
	glLinkProgram(shader_program_id);

	// Check error messages
	GLint result = GL_FALSE;
	glGetProgramiv(shader_program_id, GL_LINK_STATUS, &result);
	int info_log_length;
	glGetProgramiv(shader_program_id, GL_INFO_LOG_LENGTH, &info_log_length);
	if (info_log_length > 0)
	{
		auto error_msg = std::make_unique<char[]>(info_log_length + 1);
		glGetProgramInfoLog(shader_program_id, info_log_length, nullptr, error_msg.get());

		std::stringstream err_str;
		err_str << "Error linking shaders from files:\n"
			<< "vert: " << vertex_shader_file << "\n"
			<< "frag: " << fragment_shader_file << "\n";
		err_str << "Message:\n" << error_msg.get() << "\n";

		std::cerr << err_str.str();
#if defined(_MSC_VER)
		__debugbreak();
#endif
		exit(-1);
	}

	glDetachShader(shader_program_id, vert_shader_id);
	glDetachShader(shader_program_id, frag_shader_id);

	glDeleteShader(vert_shader_id);
	glDeleteShader(frag_shader_id);

	TMPL8_RENDERER_CHECK_ERRORS();
	return shader_program_id;
}
