#version 330 core

layout(location = 0)in vec2 _vertex;
out int texIndex;

uniform mat4 modelView;
uniform mat4 modelViewProjection;
uniform samplerBuffer heightField;

uniform uvec2 tileRowsColumns;

out vec3 viewPos;

void main()
{
    texIndex = int(_vertex.t) + int(_vertex.s) * int(tileRowsColumns.t); // texelFetch expects an integer position
    float height = texelFetch(heightField, texIndex).x;

    vec3 vertex = vec3(_vertex.x, height, _vertex.y);
    vec4 viewPos4 = modelView * vec4(vertex, 1.0);
    viewPos = viewPos4.xyz / viewPos4.w;
    // gl_PointSize = 10.0;
    gl_Position = modelViewProjection * vec4(vertex, 1.0);
}
