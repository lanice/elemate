#version 330

in vec3 v_normal[3];
in vec3 v_viewPos[3];
in vec3 v_worldPos[3];
in vec4 v_projPos[3];

out vec3 normal;
out vec3 viewPos;
out vec3 worldPos;
flat out ivec2 texCoord;

uniform vec3 tileSize;
uniform vec2 tileLeftBack;
uniform vec2 tileRowsColumns; // does not work with ivec2 (on intel at least)

layout (triangle_strip, max_vertices = 3) out;

void main()
{
    // All fragments of one quad have to use the terrain id set for their upper left vertex
    // So in our regular xz grid, use the minimum xz coordinates of all vertices and get the texCoords from them.
    // We need this to match openGL terrainIDs with physx materialIDs, which define the quad on the bottom right of a vertex.

    vec2 leftback = vec2(
        min(v_worldPos[0].xz, min(v_worldPos[1].xz, v_worldPos[2].xz)));
    vec2 normalizedTex = abs(tileLeftBack - leftback) / tileSize.xz;
    normalizedTex.y = 1-normalizedTex.y;
    ivec2 _texCoord = ivec2(normalizedTex * tileRowsColumns);
    
    for (int i=0; i < 3; ++i) {
        texCoord = _texCoord;
    
        normal = v_normal[i];
        viewPos = v_viewPos[i];
        worldPos = v_worldPos[i];
        gl_Position = v_projPos[i];
        EmitVertex();
    }
        
    EndPrimitive();
}
