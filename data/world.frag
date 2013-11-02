#version 330

in vec3 worldPos;
in vec3 viewPos;
in vec3 normal;

void main()
{
    gl_FragColor = vec4(worldPos / 2.0 + 0.5, 1.0);
}
