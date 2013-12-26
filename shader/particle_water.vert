#version 330

layout(location = 0)in vec3 vertex;

uniform mat4 viewProjection;

void main()
{
    gl_PointSize = 10.0;
    gl_Position = viewProjection * vec4(vertex, 1.0);
}
