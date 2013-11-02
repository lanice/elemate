#version 330

out vec3 worldPos;
out vec3 viewPos;
out vec3 normal;

void main()
{
    worldPos = gl_Vertex.xyz / gl_Vertex.z;
    normal = gl_Normal.xyz;
    gl_Position = ftransform(); // = gl_ModelViewMatrix * gl_Vertex; (built in variables)
    viewPos = gl_Position.xyz / gl_Position.z;
}
