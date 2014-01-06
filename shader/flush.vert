#version 330 core

layout(location = 0)in vec2 _vertex;

out vec2 v_uv;

void main()
{
    v_uv = _vertex * 0.5 + 0.5;
    gl_Position = vec4(_vertex, 0.0, 1.0);
}
