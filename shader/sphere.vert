#version 330

uniform mat4 osg_ModelViewMatrix;
uniform mat4 osg_ModelViewProjectionMatrix;

void main()
{    
    gl_Position = osg_ModelViewProjectionMatrix * gl_Vertex;
}
