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

    float height_left = (
          texelFetch(waterDepth, uv+ivec2(-1, 0), 0).x
        + texelFetch(waterDepth, uv+ivec2(-2, 0), 0).x
        )/2;

    float height_right = (
          texelFetch(waterDepth, uv+ivec2(1, 0), 0).x
        + texelFetch(waterDepth, uv+ivec2(2, 0), 0).x
        )/2;

    float height_front = (
          texelFetch(waterDepth, uv+ivec2(0, -1), 0).x
        + texelFetch(waterDepth, uv+ivec2(0, -2), 0).x
        )/2;

    float height_back = (
          texelFetch(waterDepth, uv+ivec2(0, 1), 0).x
        + texelFetch(waterDepth, uv+ivec2(0, 2), 0).x
        )/2;
    vec3 va,vb;

    if(abs(height_right-depth) < abs(height_left-depth))
        va = vec3(1.0/viewport.x, height_right - depth, 0.0);
    else
        va = vec3(1.0/viewport.x, height_left - depth, 0.0);

    if(abs(height_front-depth) < abs(height_back-depth))
        vb = vec3(0.0, height_front - depth, 1.0/viewport.y);
    else
        vb = vec3(0.0, height_back - depth, 1.0/viewport.y);

    normal = normalize(cross(vb, va));
}
