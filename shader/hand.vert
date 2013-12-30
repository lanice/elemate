#version 330 core

layout(location = 0)in vec3 _vertex;

uniform mat4 modelViewProjection;

void main()
{
    gl_Position = modelViewProjection * vec4(_vertex, 1.0) + vec4(0.0, 5.0, 0.0, 1.0);
}
