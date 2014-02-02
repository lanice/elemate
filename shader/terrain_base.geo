#version 330 core

in vec2 v_vertex[3];
in vec3 v_worldPos[3];
in vec3 v_viewPos[3];
in vec4 v_projPos[3];
in vec3 v_normal[3];

in uint v_isVisible[3];

flat out int g_texIndex;
out vec3 g_viewPos;
out vec3 g_normal;
out vec3 g_worldPos;

uniform uvec2 tileRowsColumns;

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

void main()
{
    if (!bool((v_isVisible[0] + v_isVisible[1] + v_isVisible[2])))
        return;

    // All fragments of one quad have to use the terrain id set for their upper left vertex
    // So in our regular xz grid, use the minimum xz coordinates of all vertices and get the texCoords from them.
    // We need this to match openGL terrainIDs with physx materialIDs, which define the quad on the bottom right of a vertex.

    vec2 minRowColumn = vec2(min(v_vertex[0], min(v_vertex[1], v_vertex[2])));
    // use int index (not uint) as expected by texelFetch
    int texIndex = int(minRowColumn.t) + int(minRowColumn.s) * int(tileRowsColumns.t);
    
    for (int i=0; i < 3; ++i) {
        g_texIndex = texIndex;
        
        g_normal = v_normal[i];
        
        g_worldPos = v_worldPos[i];
        
        g_viewPos = v_viewPos[i];
        gl_Position = v_projPos[i];
        EmitVertex();
    }
        
    EndPrimitive();
}