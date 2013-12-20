#version 330

layout(location = 0)in vec3 _vertex;

uniform mat4 view;
uniform mat4 viewProjection;

out vec3 viewPos;

void main()
{
    vec4 viewPos4 = view * vec4(_vertex, 1.0);
    viewPos = viewPos4.xyz / viewPos4.w;
    
    gl_Position = viewProjection * vec4(_vertex, 1.0);
}
