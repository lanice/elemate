#version 330 core

in vec2 v_uv;

uniform sampler2D waterDepth;

layout(location = 0)out vec4 fragColor;

void main()
{
	fragColor = texture(waterDepth, v_uv);
}
