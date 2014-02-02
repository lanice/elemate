#version 330 core

layout(location = 0)in vec2 _vertex;

out vec2 v_uv;

void main()
{
    v_uv 		= _vertex;
    gl_Position = vec4(v_uv,0,1);
}
