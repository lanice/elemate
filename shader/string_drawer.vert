#version 410

uniform mat4 positionTransform;
uniform mat4 textureCoordTransform;

layout (location = 0) in vec2 a_vertex;

out vec2 v_textureCoord;

void main()
{
    vec4 textureCoord = textureCoordTransform * vec4(a_vertex, 0.0, 1.0);
    vec4 position = positionTransform * vec4(a_vertex, 0.0, 1.0);

    v_textureCoord = textureCoord.xy / textureCoord.w;
    gl_Position = position;
}
