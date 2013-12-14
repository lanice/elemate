#version 330

in vec2 v_uv;

uniform sampler2D colorBuffer;

layout(location = 0)out vec4 fragColor;

void main()
{
    fragColor = texture(colorBuffer, v_uv);
}
