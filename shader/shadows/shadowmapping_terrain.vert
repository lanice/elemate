#version 330 core

layout(location = 0)in vec2 _vertex;

uniform mat4 modelViewProjection;

uniform mat4 lightBiasMVP;

uniform samplerBuffer heightField;

uniform int tileSamplesPerAxis;
uniform ivec2 rowColumnOffset;

out vec4 v_shadowCoord;

void main()
{
    ivec2 rowColumn = ivec2(_vertex) + rowColumnOffset;
    int texIndex = rowColumn.t + rowColumn.s * tileSamplesPerAxis;

    vec4 vertex = vec4(float(rowColumn.x), texelFetch(heightField, texIndex).x, float(rowColumn.y), 1.0);
    
    gl_Position = modelViewProjection * vertex;
    
    v_shadowCoord = lightBiasMVP * vertex;
}
