#version 330

in vec2 v_vertex[3];
in vec3 v_viewPos[3];
in vec3 v_worldPos[3];
in vec4 v_projPos[3];

out vec3 g_normal;
out vec3 g_viewPos;
out vec3 g_worldPos;
out int g_texIndex;

uniform uvec2 tileRowsColumns;

layout (triangle_strip, max_vertices = 3) out;

void main()
{
    // All fragments of one quad have to use the terrain id set for their upper left vertex
    // So in our regular xz grid, use the minimum xz coordinates of all vertices and get the texCoords from them.
    // We need this to match openGL terrainIDs with physx materialIDs, which define the quad on the bottom right of a vertex.

    vec2 minRowColumn = vec2(min(v_vertex[0], min(v_vertex[1], v_vertex[2])));
    int texIndex = int(minRowColumn.t) + int(minRowColumn.s) * int(tileRowsColumns.t);
    
    for (int i=0; i < 3; ++i) {
        g_texIndex = texIndex;
    
        // TODO calculate normals
        g_normal = vec3(0.0, 1.0, 0.0);
        
        g_viewPos = v_viewPos[i];
        g_worldPos = v_worldPos[i];
        gl_Position = v_projPos[i];
        EmitVertex();
    }
        
    EndPrimitive();
}