#version 330 core

in vec2 g_relPos;
in vec4 g_absPos;
in vec2 factor;

void main()
{
	if(length(g_relPos) > 1)
		discard;
    gl_FragDepth = (g_absPos.z - sqrt(1.0 - dot(g_relPos*factor, g_relPos*factor)))/g_absPos.w;
}
