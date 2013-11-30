#version 330

// writing int to attrib array and use them as float here seems to work :-D
layout(location = 2) in float terrainTypeID;
// you shall not int? works only with float for me, int produces some kind of unpredictable values
flat out float v_terrainTypeID;

out vec3 v_normal;
out vec3 v_viewPos;
out vec3 v_worldPos;
out vec4 v_projPos;

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
    v_terrainTypeID = terrainTypeID;
    v_worldPos = osgTerrainToOpenGl * (gl_Vertex.xyz / gl_Vertex.w);
	v_normal = normalize(osgTerrainToOpenGl * gl_Normal);
    vec4 viewPos4 = (osg_ModelViewMatrix * gl_Vertex);
    v_viewPos = viewPos4.xyz / viewPos4.w;
    v_projPos = osg_ModelViewProjectionMatrix * gl_Vertex;
}