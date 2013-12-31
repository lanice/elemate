#version 330 core

layout(location = 0)in vec3 _vertex;
layout(location = 1)in vec3 _normal;

uniform mat4 modelView;
uniform mat4 modelViewProjection;
uniform mat4 rotate;

out vec3 v_normal;
out vec3 v_viewPos;

void main()
{
    v_normal = (rotate * vec4(_normal, 1.0)).xyz;
    vec4 viewPos4 = modelView * vec4(_vertex, 1.0);
    v_viewPos = viewPos4.xyz / viewPos4.w;
    gl_Position = modelViewProjection * vec4(_vertex, 1.0);
}
