#version 330 core

layout(location = 0)in vec2 _vertex;

uniform mat4 depthMVP;

uniform samplerBuffer heightField0;
uniform samplerBuffer heightField1;

uniform uvec2 tileRowsColumns;

void main()
{
    int texIndex = int(_vertex.t) + int(_vertex.s) * int(tileRowsColumns.t);
    float height = max(texelFetch(heightField0, texIndex).x,
                       texelFetch(heightField1, texIndex).x);
    
    gl_Position = depthMVP * vec4(_vertex.x, height, _vertex.y, 1.0);
}
