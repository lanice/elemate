#version 330

// writing int to attrib array and use them as float here seems to work :-D
layout(location = 2) in float terrainTypeID;
// you shall not int? works only with float for me, int produces some kind of unpredictable values
flat out float f_terrainTypeID;

out vec3 normal;
out vec3 position;
out vec2 screenPos;
out vec3 worldPos;

uniform mat4 osg_ModelViewMatrix;
uniform mat4 osg_ModelViewProjectionMatrix;

void main(void) 
{
    f_terrainTypeID = terrainTypeID;
    worldPos = gl_Vertex.xyz;
	normal = normalize(gl_Normal);
    gl_Position = osg_ModelViewProjectionMatrix * gl_Vertex;
    position = vec3(osg_ModelViewMatrix * gl_Vertex);
    screenPos = (gl_Position.xz * 0.5) + 0.5;
}