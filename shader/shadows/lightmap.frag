#version 330 core

float linearize(float depth);

void main()
{
    // gl_FragDepth = linearize(gl_FragCoord.z);
}
