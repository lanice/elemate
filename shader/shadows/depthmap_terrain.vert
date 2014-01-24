#version 330 core

layout(location = 0)in vec2 _vertex;

uniform mat4 depthMVP;

uniform samplerBuffer heightField;

uniform uvec2 tileRowsColumns;

void main()
{
    int texIndex = int(_vertex.t) + int(_vertex.s) * int(tileRowsColumns.t);
    
    gl_Position = depthMVP * vec4(_vertex.x, texelFetch(heightField, texIndex).x, _vertex.y, 1.0);
}
