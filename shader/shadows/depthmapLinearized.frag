#version 330 core

float linearize(float depth);

layout(location = 0)out uint f_elementIndex;

void main()
{
    f_elementIndex = 1u;

    gl_FragDepth = linearize(gl_FragCoord.z);
}
