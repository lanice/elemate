#version 330 core

in vec2 g_vertex;

void main()
{
	if(length(g_vertex) > 1)
		discard;
}
