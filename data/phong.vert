#version 330

out vec3 normal;
out vec3 position;

uniform mat4 osg_ModelViewMatrix;
uniform mat4 osg_ModelViewProjectionMatrix;

void main(void) 
{
	normal = normalize(gl_Normal);
    gl_Position = osg_ModelViewProjectionMatrix * gl_Vertex;
    position = vec3(osg_ModelViewMatrix * gl_Vertex);
}