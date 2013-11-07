#version 330

in vec3 worldPos;
in vec3 viewPos;
in vec3 normal;
uniform vec3 namederuniform;

void main()
{
    // gl_FragColor = vec4(worldPos / 2.0 + 0.5, 1.0);
    gl_FragColor = vec4(namederuniform, 1.0);
}
