#version 330 core

in vec2 v_uv;

uniform sampler2D waterDepth;
uniform ivec2 viewport;

layout(location = 0)out float depthValue;

void main()
{
    ivec2 uv = ivec2(v_uv*vec2(viewport));
    
    depthValue = texelFetch(waterDepth, uv, 0).r;
}
