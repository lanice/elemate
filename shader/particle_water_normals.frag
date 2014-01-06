#version 330 core

in vec2 v_uv;

const float dist = 0.01;

uniform sampler2D waterDepth;
uniform ivec2 viewport;

layout(location = 0)out vec3 normal;

void main()
{
	ivec2 uv = ivec2(v_uv*vec2(viewport));
	float depth = texelFetch(waterDepth, uv, 0).r;

	float height_left = texelFetch(waterDepth, uv+ivec2(-1, 0), 0).x;
    float height_right = texelFetch(waterDepth, uv+ivec2(1, 0), 0).x;
    float height_front = texelFetch(waterDepth, uv+ivec2(0, -1), 0).x;
    float height_back = texelFetch(waterDepth, uv+ivec2(0, 1), 0).x;
    vec3 va = vec3(2.0/viewport.x, height_right - height_left, 0.0);
    vec3 vb = vec3(0.0, height_back - height_front, 2.0/viewport.y);
    normal = normalize(cross(vb, va));
}
