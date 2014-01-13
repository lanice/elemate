#version 330 core

in vec2 g_relPos;
in vec4 g_absPos;
in float size;

void main()
{
	if(length(g_relPos) > 1)
		discard;
    gl_FragDepth = (g_absPos.z - size*sqrt(1.0 - dot(g_relPos.xy, g_relPos.xy)))/g_absPos.w;
}
