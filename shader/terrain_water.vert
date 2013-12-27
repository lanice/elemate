#version 330 core

layout(location = 0)in vec2 _vertex;

out vec3 v_viewPos;

uniform mat4 modelView;
uniform mat4 modelViewProjection;

uniform samplerBuffer heightField;

uniform uvec2 tileRowsColumns;

void main()
{
    int texIndex = int(_vertex.t) + int(_vertex.s) * int(tileRowsColumns.t);
    float height = texelFetch(heightField, texIndex).x;
    
    vec4 vertex = vec4(_vertex.x, height, _vertex.y, 1.0);
    
    vec4 viewPos4 = modelView * vertex;
    v_viewPos = viewPos4.xyz / viewPos4.w;
    
    gl_Position = modelViewProjection * vertex;
}
