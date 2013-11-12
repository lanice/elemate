#version 110

varying vec3 worldPos;
varying vec3 viewPos;
varying vec3 normal;
uniform vec3 namederuniform;

void main()
{
    gl_FragColor = mix(vec4(worldPos / 2.0 + 0.5, 1.0), vec4(namederuniform, 1.0), 0.3);
}
