#version 330 core

layout(location = 0)in vec2 _vertex;

uniform float time_mod;
uniform float ratio;

out vec2 v_uv;

void main()
{
    v_uv 		= _vertex;
    gl_Position = vec4(
    				0.75+0.25*_vertex.x, 
    				0.85+time_mod*0.85+0.1*_vertex.y*ratio,
    				0,
    				1);
}
