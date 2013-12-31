#version 330 core

layout(location = 0)in vec3 _vertex;
layout(location = 1)in vec3 _normal;

uniform mat4 modelViewProjection;

out vec3 v_normal;

void main()
{
    v_normal = _normal;
    gl_Position = modelViewProjection * vec4(_vertex, 1.0) + vec4(0.0, 5.0, 0.0, 1.0);
}
