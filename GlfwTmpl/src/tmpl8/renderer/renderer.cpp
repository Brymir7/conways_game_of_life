#include <tmpl8/renderer/renderer.hpp>

#include <iostream>
#include <chrono>
#include <sstream>

#include <glm/glm.hpp>

#include <tmpl8/renderer/includes.hpp>
#include <tmpl8/renderer/shader_loader.hpp>

#include <tmpl8/game_class.hpp>
#include <config.hpp>

namespace
{
	struct blit_vertex
	{
		glm::vec2 pos_vs;
		glm::vec2 texcoord;
	};
}

void   glfw_key_callback(GLFWwindow* wnd, int key, int scancode, int action, int mods);
void   glfw_char_callback(GLFWwindow* wnd, unsigned int letter);
void   glfw_mouse_callback(GLFWwindow* wnd, int button, int action, int mods);
void   glfw_move_callback(GLFWwindow* wnd, double x, double y);

void   glfw_error_handler(int error, const char* description);
GLuint init_blit_shader();
void   free_blit_shader(GLuint shader_program_id);
void   init_blit_vertices(GLuint& out_vao, GLuint& out_vbo);
void   free_blit_vertices(GLuint& ref_vao, GLuint& ref_vbo);
GLuint init_blit_texture();
void   updt_blit_texture(GLuint tex_id, glm::u32* pixel_data, GLsizei width, GLsizei height, GLint stride);
void   free_blit_texture(GLuint& tex_id);
void   draw_blit(GLuint shader_id, GLuint vao, GLuint tex_id);

namespace tmpl8
{
	namespace renderer
	{
		int start_game_loop()
		{
			static_assert(config::screen_width > 0 && config::screen_height > 0, "Window size must be positive.");

			if (glfwInit() != GL_TRUE)
			{
				std::cerr << "Failed to initialize GLFW." << std::endl;
				return -1;
			}

			glfwSetErrorCallback(glfw_error_handler);

			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
			glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
			glfwWindowHint(GLFW_STENCIL_BITS, 0);
			glfwWindowHint(GLFW_DEPTH_BITS, 0);

			GLFWwindow* wnd = glfwCreateWindow(config::screen_width << config::scale_shift, config::screen_height << config::scale_shift, config::screen_title, nullptr, nullptr);
			glfwMakeContextCurrent(wnd);

			if (wnd == nullptr)
			{
				std::cerr << "Failed to create GLFW window" << std::endl;
				glfwTerminate();
				return -1;
			}

			glfwSetKeyCallback(wnd, glfw_key_callback);
			glfwSetCharCallback(wnd, glfw_char_callback);
			glfwSetMouseButtonCallback(wnd, glfw_mouse_callback);
			glfwSetCursorPosCallback(wnd, glfw_move_callback);

			if (!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress)))
			{
				std::cerr << "Failed to initialize OpenGL context" << std::endl;
				return -1;
			}

			GLuint shader_program_id = init_blit_shader();
			GLuint vao, vbo;
			init_blit_vertices(vao, vbo);

			GLuint blit_texure_id = init_blit_texture();

			glfwSwapInterval(config::use_vsync ? 1 : 0);

			glDisable(GL_STENCIL_TEST);
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			glClearColor(0.15f, 0.1f, 0.1f, 0.0f);

			std::stringstream window_title_buffer;
			{
				surface screen(config::screen_width, config::screen_height);
				game_class game(screen);

				glfwSetWindowUserPointer(wnd, &game);

				using clock = std::chrono::high_resolution_clock;
				using std::chrono::duration_cast;


				float frame_time = 0.f;
				while (!glfwWindowShouldClose(wnd))
				{
					auto frame_start = clock::now();
					glClear(GL_COLOR_BUFFER_BIT);

					TMPL8_RENDERER_CHECK_ERRORS();

					game.tick(frame_time);

					TMPL8_RENDERER_CHECK_ERRORS();

					updt_blit_texture(blit_texure_id, screen.buffer(), config::screen_width, config::screen_height, 0);
					draw_blit(shader_program_id, vao, blit_texure_id);

					TMPL8_RENDERER_CHECK_ERRORS();

					// Swap buffers
					glfwSwapBuffers(wnd);
					glfwPollEvents();

					frame_time = duration_cast<std::chrono::duration<float>>(clock::now() - frame_start).count();
					frame_time = std::min(frame_time, 0.1f);
					window_title_buffer.seekp(0);
					window_title_buffer << config::screen_title << " - FPS: " << 1.0 / frame_time;
					glfwSetWindowTitle(wnd, window_title_buffer.str().c_str());
				}
			}

			free_blit_texture(blit_texure_id);
			free_blit_vertices(vao, vbo);
			free_blit_shader(shader_program_id);

			glfwTerminate();

			return 0;
		}
	}
}

void glfw_key_callback(GLFWwindow* wnd, int glfw_key, int /*scancode*/, int action, int glfw_mods)
{
	const auto key = static_cast<tmpl8::key>(glfw_key);
	const auto mods = static_cast<tmpl8::modifiers>(glfw_mods);

#if defined(_MSC_VER)
#pragma warning (push)
#pragma warning (disable: 4127)
#endif
	if (config::exit_on_escape && key == tmpl8::key::escape && mods == tmpl8::modifiers::none && action == GLFW_PRESS)
		return glfwSetWindowShouldClose(wnd, GL_TRUE);
#if defined(_MSC_VER)
#pragma warning (pop)
#endif

	tmpl8::game_class& game = *static_cast<tmpl8::game_class*>(glfwGetWindowUserPointer(wnd));
	switch (action)
	{
	case GLFW_PRESS:
		game.key_down(key, mods);
		break;
	case GLFW_RELEASE:
		game.key_up(key, mods);
		break;
	case GLFW_REPEAT:
		game.key_repeat(key, mods);
		break;
	}
}

void glfw_char_callback(GLFWwindow* wnd, unsigned int letter)
{
	static_cast<tmpl8::game_class*>(glfwGetWindowUserPointer(wnd))->key_char(letter);
}

void glfw_mouse_callback(GLFWwindow* wnd, int glfw_button, int action, int glfw_mods)
{
	tmpl8::game_class& game = *static_cast<tmpl8::game_class*>(glfwGetWindowUserPointer(wnd));
	const auto button = static_cast<tmpl8::mouse_button>(glfw_button);
	const auto mods = static_cast<tmpl8::modifiers>(glfw_mods);
	switch (action)
	{
	case GLFW_PRESS:
		game.mouse_down(button, mods);
		break;
	case GLFW_RELEASE:
		game.mouse_up(button, mods);
		break;
	}
}

void glfw_move_callback(GLFWwindow* wnd, double x, double y)
{
	constexpr size_t scale = 1 << config::scale_shift;
	constexpr size_t offset = scale >> 1;
	static_cast<tmpl8::game_class*>(glfwGetWindowUserPointer(wnd))->mouse_move(
		(static_cast<int32_t>(x) + offset) / scale,
		static_cast<int32_t>(config::screen_height) -
		(static_cast<int32_t>(y) + offset) / scale);
}

void glfw_error_handler(int error, const char* description)
{
	std::cerr << "GLFW error " << error << ": " << description << "\n";
}

GLuint init_blit_shader()
{
	using tmpl8::renderer::create_shader_program;
	GLuint shaderProgId = create_shader_program("shaders/blit.vert", "shaders/blit.frag");
	GLint  uniformTex = glGetUniformLocation(shaderProgId, "Tex");

	glUseProgram(shaderProgId);
	glUniform1i(uniformTex, 0);
	glUseProgram(0);

	return shaderProgId;
}

void free_blit_shader(GLuint shader_id)
{
	glDeleteProgram(shader_id);
}

void init_blit_vertices(GLuint& vao, GLuint& vbo)
{
	// Generate the Vertex Buffer Object
	glGenBuffers(1, &vbo);

	// Generate Vertex Array Object
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Position viewport space
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(/* layout index */0, /* size */2, /* type */GL_FLOAT, /* normalized */GL_FALSE,
		/* stride */sizeof(blit_vertex), /* offset */reinterpret_cast<void*>(offsetof(blit_vertex, pos_vs)));

	// UV (U, V)
	glVertexAttribPointer(/* layout index */1, /* size */2, /* type */GL_FLOAT, /* normalized */GL_FALSE,
		/* stride */sizeof(blit_vertex), /* offset */reinterpret_cast<void*>(offsetof(blit_vertex, texcoord)));

	// Enable the attributes
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	// Upload vertex data
	static const blit_vertex vertexData[6] =
	{
		{ { -1.0f, -1.0f }, { 0.f, 0.f } }, // BL
		{ {  1.0f, -1.0f }, { 1.f, 0.f } }, // BR
		{ { -1.0f,  1.0f }, { 0.f, 1.f } }, // TL
		{ { -1.0f,  1.0f }, { 0.f, 1.f } }, // TL
		{ {  1.0f, -1.0f }, { 1.f, 0.f } }, // BR
		{ {  1.0f,  1.0f }, { 1.f, 1.f } }  // TR
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(blit_vertex) * 6, vertexData, GL_STATIC_DRAW);

	// Unbind
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void free_blit_vertices(GLuint& vao, GLuint& vbo)
{
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}

GLuint init_blit_texture()
{
	GLuint texId;
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, config::scale_interpolate ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, config::scale_interpolate ? GL_LINEAR : GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);
	return texId;
}

void updt_blit_texture(GLuint tex_id, glm::u32* pixel_data, GLsizei width, GLsizei height, GLint stride)
{
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, stride, GL_BGRA, GL_UNSIGNED_BYTE, pixel_data);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void free_blit_texture(GLuint& tex_id)
{
	glDeleteTextures(1, &tex_id);
}

void draw_blit(GLuint shader_id, GLuint vao, GLuint tex_id)
{
	glUseProgram(shader_id);
	glBindVertexArray(vao);

	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, tex_id);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);
	glUseProgram(0);
}
