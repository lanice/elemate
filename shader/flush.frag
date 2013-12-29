#version 330 core

in vec2 v_uv;

uniform sampler2D sceneColor;
uniform sampler2D sceneDepth;
uniform sampler2D waterDepth;

layout(location = 0)out vec4 fragColor;

void main()
{
    float sceneZ = texture(sceneDepth, v_uv).r;
    float waterZ = texture(waterDepth, v_uv).r;
    if (sceneZ > waterZ)
        fragColor = vec4(0, 0, 1, 0.5);
    else
        fragColor = texture(sceneColor, v_uv);
    // fragColor = texture(waterDepth, v_uv) * 0.5;
    // fragColor = texture(depthBuffer, v_uv);
}
