#version 330 core

in vec2 v_uv;

uniform sampler2D waterDepth;
uniform ivec2 viewport;

layout(location = 0)out float depthValue;

void main()
{
    ivec2 uv = ivec2(v_uv*vec2(viewport));

    float sum = 0;
    float blurSize = 1.0;
    sum += texelFetch(waterDepth, ivec2(uv.x - 4.0*blurSize, uv.y), 0).r * 0.05;
    sum += texelFetch(waterDepth, ivec2(uv.x - 3.0*blurSize, uv.y), 0).r * 0.09;
    sum += texelFetch(waterDepth, ivec2(uv.x - 2.0*blurSize, uv.y), 0).r * 0.12;
    sum += texelFetch(waterDepth, ivec2(uv.x - blurSize, uv.y), 0).r * 0.15;
    sum += texelFetch(waterDepth, ivec2(uv.x, uv.y), 0).r * 0.16;
    sum += texelFetch(waterDepth, ivec2(uv.x + blurSize, uv.y), 0).r * 0.15;
    sum += texelFetch(waterDepth, ivec2(uv.x + 2.0*blurSize, uv.y), 0).r * 0.12;
    sum += texelFetch(waterDepth, ivec2(uv.x + 3.0*blurSize, uv.y), 0).r * 0.09;
    sum += texelFetch(waterDepth, ivec2(uv.x + 4.0*blurSize, uv.y), 0).r * 0.05;

    depthValue = sum;
}
