#version 330

out vec3 worldPos;
out vec3 viewPos;
out vec3 normal;

//uniform mat4 osg_ModelViewMatrix;
uniform mat4 osg_ModelViewProjectionMatrix;

void main()
{
    worldPos = gl_Vertex.xyz / gl_Vertex.w;
    normal = gl_Normal.xyz;
    gl_Position = osg_ModelViewProjectionMatrix * gl_Vertex;
    viewPos = gl_Position.xyz / gl_Position.w;
}
