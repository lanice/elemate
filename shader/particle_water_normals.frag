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
    float height_up = texelFetch(source, uv+ivec2(0, 1), 0).x;
    float height_down = texelFetch(source, uv+ivec2(0, -1), 0).x;

    vec3 va,vb;

    va = mix(
            vec3(0.001*(0.01+height_right), 0.0, height_right - depth),
            vec3(0.001*(0.01+height_left), 0.0, depth - height_left),
            step(
                abs(height_left-depth),
                abs(height_right-depth)
            )
    );
    vb = mix(
        vec3(0.0, 0.001*(0.01+height_up), depth - height_up),
        vec3(0.0, 0.001*(0.01+height_down), height_down - depth),
        step(
            abs(height_down-depth),
            abs(height_up-depth)
        )
    );

    normal = normalize(cross(va, vb));
}
