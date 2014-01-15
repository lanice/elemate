#version 330 core

in vec2 v_uv;

const float dist = 0.01;

uniform sampler2D source;
uniform ivec2 viewport;

layout(location = 0)out vec3 normal;

void main()
{
    ivec2 uv = ivec2(v_uv*vec2(viewport));
    float depth = texelFetch(source, uv, 0).r;

    float height_left = texelFetch(source, uv+ivec2(-1, 0), 0).x;
    float height_right = texelFetch(source, uv+ivec2(1, 0), 0).x;
    float height_front = texelFetch(source, uv+ivec2(0, -1), 0).x;
    float height_back = texelFetch(source, uv+ivec2(0, 1), 0).x;

    vec3 va,vb;

    if(abs(height_right-depth) < abs(height_left-depth))
        va = vec3(1.0/viewport.x, height_right - depth, 0.0);
    else
        va = vec3(1.0/viewport.x, depth - height_left, 0.0);

    if(abs(height_front-depth) < abs(height_back-depth))
        vb = vec3(0.0, height_front - depth, 1.0/viewport.y);
    else
        vb = vec3(0.0, depth - height_back, 1.0/viewport.y);

    normal = normalize(cross(vb, va));
}
