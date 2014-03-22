#version 330 core

layout(location = 0)in vec2 _vertex;

uniform float x;
uniform float y;
uniform float ratio;
uniform float width;

out vec2 v_uv;

void main()
{
    v_uv      = _vertex;
    gl_Position = vec4(
              x+width * _vertex.x, 
              y+width *  _vertex.y*ratio,
              0,
              1);
}
