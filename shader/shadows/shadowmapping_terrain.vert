#version 330 core

layout(location = 0)in vec2 _vertex;

uniform mat4 modelViewProjection;

uniform mat4 lightBiasMVP;

uniform samplerBuffer heightField;

uniform uvec2 tileRowsColumns;

out vec4 v_shadowCoord;

void main()
{
    int texIndex = int(_vertex.t) + int(_vertex.s) * int(tileRowsColumns.t);

    vec4 vertex = vec4(_vertex.x, texelFetch(heightField, texIndex).x, _vertex.y, 1.0);
    
    gl_Position = modelViewProjection * vertex;
    
    v_shadowCoord = lightBiasMVP * vertex;
}
