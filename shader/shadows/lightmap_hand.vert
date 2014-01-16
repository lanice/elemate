#version 330 core

layout(location = 0)in vec3 _vertex;

uniform mat4 lightMVP;

void main()
{
    gl_Position = lightMVP * vec4(_vertex, 1.0);
}
