#version 330 core

in vec3 v_normal;

layout(location = 0)out vec4 fragColor;

void main()
{
    fragColor = vec4(v_normal, 1.0);
    // fragColor = vec4(0.8, 0.8, 0.8, 1.0);
}
