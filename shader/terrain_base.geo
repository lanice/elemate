#version 330 core

in ivec2 v_vertex[3];
in vec3 v_worldPos[3];
in vec3 v_viewPos[3];
in vec4 v_projPos[3];
in vec3 v_normal[3];
in float v_temperature[3];

out vec3 g_viewPos;
out vec3 g_normal;
out vec3 g_worldPos;
out vec2 g_rowColumn;
out vec2 g_quadRelativePos;
out float g_temperature;

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

void main()
{
    // All fragments of one quad have to use the terrain id set for their upper left vertex
    // So in our regular xz grid, use the minimum xz coordinates of all vertices and get the texCoords from them.
    // We need this to match openGL terrainIDs with physx materialIDs, which define the quad on the bottom right of a vertex.

    ivec2 minRowColumn = ivec2(min(v_vertex[0], min(v_vertex[1], v_vertex[2])));
    
    for (int i=0; i < 3; ++i) {
        g_normal = v_normal[i];
        g_worldPos = v_worldPos[i];
        g_viewPos = v_viewPos[i];
        g_quadRelativePos = vec2(   // relative position of minRowColumn vertex is (0,0)
            bvec2(minRowColumn.s != v_vertex[i].s,      // false = 0.0, true = 1.0
                  minRowColumn.t != v_vertex[i].t));
        g_rowColumn = vec2(minRowColumn) + g_quadRelativePos - vec2(0.5);
        g_temperature = v_temperature[i];
        gl_Position = v_projPos[i];
        EmitVertex();
    }
        
    EndPrimitive();
}