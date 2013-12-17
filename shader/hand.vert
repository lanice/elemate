#version 330

out vec3 worldPos;
out vec3 normal;
out vec3 viewPos;

uniform mat4 osg_ModelViewMatrix;
uniform mat4 osg_ModelViewProjectionMatrix;

void main()
{
    vec4 viewPos4 = osg_ModelViewMatrix * gl_Vertex;
    viewPos = viewPos4.xyz / viewPos4.w;
    normal = normalize(gl_Normal);
    gl_Position = osg_ModelViewProjectionMatrix * gl_Vertex;
}
