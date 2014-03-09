#version 330 core

in float g_height;

float linearize(float depth);

layout(location = 0)out uint f_elementIndex;

void main()
{
    f_elementIndex = uint(mix(1, 2, step(0.01, g_height)));

    gl_FragDepth = linearize(gl_FragCoord.z);
}
