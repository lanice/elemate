#version 330 core

uniform sampler2DShadow lightMap;


in vec4 v_shadowCoord;

layout(location = 0)out float visibility;

float zBias = 0.007;

void main()
{
    visibility = max(
        texture(lightMap, vec3(v_shadowCoord.xy, (v_shadowCoord.z - zBias) / v_shadowCoord.w)),
        0.5);
}
