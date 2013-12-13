#version 330

uniform mat4 osg_ModelViewProjectionMatrix;

void main()
{
    gl_Position = osg_ModelViewProjectionMatrix * vec4(gl_Vertex.xyz, 1.0);
}
