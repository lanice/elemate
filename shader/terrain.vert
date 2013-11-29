#version 330

// writing int to attrib array and use them as float here seems to work :-D
layout(location = 2) in float terrainTypeID;
// you shall not int? works only with float for me, int produces some kind of unpredictable values
flat out float f_terrainTypeID;

out vec3 normal;
out vec3 viewPos;
out vec2 screenPos;
out vec3 worldPos;

uniform mat4 osg_ModelViewMatrix;
uniform mat4 osg_ModelViewProjectionMatrix;

/** Base transform: osg to OpenGL 
    We need this, because osg creates terrains in xy plane, but we want them in xz.
    osg_* matrices already contain this one, but we need it to use world coordinates. */
mat3 osgTerrainToOpenGl = mat3(
    1,0,0,  // column 1
    0,0,-1, // column 2 ...
    0,1,0);

void main(void) 
{
    f_terrainTypeID = terrainTypeID;
    worldPos = osgTerrainToOpenGl * (gl_Vertex.xyz / gl_Vertex.w);
	normal = normalize(osgTerrainToOpenGl * gl_Normal);
    viewPos = vec3(osg_ModelViewMatrix * gl_Vertex);
    gl_Position = osg_ModelViewProjectionMatrix * gl_Vertex;
    screenPos = (gl_Position.xz * 0.5) + 0.5;
}