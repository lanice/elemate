#version 330 core

// layout(location = 0)out float depthValue;

float linearize(float depth);

void main()
{
    // gl_FragDepth = linearize(gl_FragCoord.z);
    // depthValue = linearize(gl_FragCoord.z);
}
