#version 330 core

in float v_heightDiff;

uniform bool baseTileCompare;

float linearize(float depth);

void main()
{
    if (baseTileCompare)
        if (v_heightDiff < 0)
            discard;

    gl_FragDepth = linearize(gl_FragCoord.z);
}
