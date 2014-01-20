#version 330 core

in vec2 g_relPos;
flat in float g_zDiff;

uniform float particleSize;

float linearize(float depth);

void main()
{
	if(length(g_relPos) > 1.0)
		discard;

    gl_FragDepth = gl_FragCoord.z - sqrt(1.0 - dot(g_relPos.xy, g_relPos.xy)) * g_zDiff * gl_FragCoord.w;
}
