#version 330 core

layout(location = 0) in vec2 v_pos_vs;
layout(location = 1) in vec2 v_texcoord;

out vec2 f_texcoord;

void main()
{
	gl_Position = vec4(v_pos_vs, 0.f, 1.f);
	f_texcoord  = v_texcoord;
}
