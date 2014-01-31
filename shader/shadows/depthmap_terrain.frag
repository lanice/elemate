#version 330 core

in float v_heightDiff;

uniform bool baseTileCompare;

void main()
{ 
    if (baseTileCompare)
        if (v_heightDiff < 0)
            discard;
}
