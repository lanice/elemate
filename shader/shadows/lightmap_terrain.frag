#version 330 core

layout(location = 0)out float depthValue;

float linearize(float depth);

void main()
{
    // depthValue = linearize(gl_FragCoord.z);
    depthValue = gl_FragCoord.z;
}
