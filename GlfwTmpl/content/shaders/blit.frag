#version 330 core

in vec2 f_texcoord;

out vec4 out_color;

uniform sampler2D Tex;

void main()
{
	out_color = vec4(texture2D(Tex, f_texcoord).rgb, 1.0);
}
