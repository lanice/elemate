#version 330 core

layout(location = 0)in vec2 _vertex;

out ivec2 v_vertex;

uniform ivec2 rowColumnOffset;

void main()
{
    v_vertex = ivec2(_vertex) + rowColumnOffset;
}
