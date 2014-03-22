#version 330 core

layout(location = 0)in vec2 _vertex;

uniform float time_mod;
uniform float ratio;
uniform float scale;
uniform float x;
uniform float y;

out vec2 v_uv;

void main()
{
    v_uv = _vertex;
    gl_Position = vec4(
            x+0.05+0.25*_vertex.x*scale, 
            y-0.05+time_mod*0.85+0.1*_vertex.y*ratio*scale,
            0,
            1);
}
