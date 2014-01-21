#version 330 core

layout(location = 0)in vec3 _vertex;
out vec3 v_vertex;

void main()
{
    v_vertex = _vertex + vec3(0,0.2,0);
}
