#version 330

layout(location = 0)in vec2 _vertex;

// uniform mat4 view;
uniform mat4 viewProjection;
uniform mat4 modelViewProjection;

// out vec3 viewPos;

void main()
{
    vec3 vertex = vec3(_vertex.x, 0.0, _vertex.y);
    // vec4 viewPos4 = view * vec4(vertex, 1.0);
    // viewPos = viewPos4.xyz / viewPos4.w;
    gl_PointSize = 10.0;
    gl_Position = modelViewProjection * vec4(vertex, 1.0);
}
