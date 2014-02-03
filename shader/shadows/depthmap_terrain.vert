#version 330 core

layout(location = 0)in vec2 _vertex;

out vec2 v_vertex;

void main()
{
    v_vertex = _vertex;
}
