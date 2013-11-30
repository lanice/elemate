#version 330

flat in float v_terrainTypeID[3];
in vec3 v_normal[3];
in vec3 v_viewPos[3];
in vec3 v_worldPos[3];
in vec4 v_projPos[3];

out vec3 normal;
out vec3 viewPos;
out vec3 worldPos;
flat out float f_terrainTypeID;

layout (triangle_strip, max_vertices = 3) out;

void main()
{
    // the fragments provoked by this primitive have to use the terrainTypeID of the upper left vertex
    // We need this to match openGL terrainIDs with physx materialIDs, which define the quad on the bottom right of a vertex.
    // All vertices wordPos are in a regular xz grid.
    // "...the diagonal of the quad will run from this sample to the opposite vertex..." (PxHeightFieldSample.h:89) (tesselation flag not set)

    int provokingVertex;
    int left1id = (v_worldPos[0].x < v_worldPos[1].x) ? 0 : 1;
    int left2id = (v_worldPos[left1id].x < v_worldPos[2].x) ? left1id : 2;
    if (left1id == left2id) { // we are in the upper triangle
        provokingVertex = left1id;
    } else { // we are in the lower triangle
        provokingVertex = (v_worldPos[left1id].z < v_worldPos[left2id].z) ? left1id : left2id;
    }
    
    for (int i=0; i < 3; ++i) {
        f_terrainTypeID = v_terrainTypeID[provokingVertex];
    
        normal = v_normal[i];
        viewPos = v_viewPos[i];
        worldPos = v_worldPos[i];
        gl_Position = v_projPos[i];
        EmitVertex();
    }
        
    EndPrimitive();
}
