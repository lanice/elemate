#version 330 core

layout(location = 0)in vec2 _vertex;

uniform mat4 depthMVP;

uniform samplerBuffer heightField;
uniform samplerBuffer baseHeightField;

uniform uvec2 tileRowsColumns;

uniform bool baseTileCompare;

out float v_heightDiff;

void main()
{
    int texIndex = int(_vertex.t) + int(_vertex.s) * int(tileRowsColumns.t);
    
    float height = texelFetch(heightField, texIndex).x;
    
    if (baseTileCompare)
        v_heightDiff = height - texelFetch(baseHeightField, texIndex).x;
    
    gl_Position = depthMVP * vec4(_vertex.x, height, _vertex.y, 1.0);
}
