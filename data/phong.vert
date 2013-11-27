#version 330

out vec3 normal;
out vec3 position;
out vec2 screenPos;
out vec3 worldPos;

uniform mat4 osg_ModelViewMatrix;
uniform mat4 osg_ModelViewProjectionMatrix;

void main(void) 
{
    worldPos = gl_Vertex.xyz;
	normal = normalize(gl_Normal);
    gl_Position = osg_ModelViewProjectionMatrix * gl_Vertex;
    position = vec3(osg_ModelViewMatrix * gl_Vertex);
    screenPos = (gl_Position.xz * 0.5) + 0.5;
}