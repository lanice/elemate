#version 330 core

in ivec2 v_vertex[3];

uniform mat4 depthMVP;

uniform samplerBuffer heightField;
uniform samplerBuffer baseHeightField;

uniform int tileSamplesPerAxis;

uniform bool baseTileCompare;

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

void main()
{
    vec4 positions[3];
    bool visibleTriangle = false;
    
    for (int i=0; i < 3; ++i) {
        int texIndex = v_vertex[i].t + v_vertex[i].s * tileSamplesPerAxis;
        float height = texelFetch(heightField, texIndex).x;
        positions[i] = depthMVP * vec4(float(v_vertex[i].x), height, float(v_vertex[i].y), 1.0);
        
        vec3 normProjPos = positions[i].xyz / positions[i].w;
        bool isOnTop = true;
        if (baseTileCompare)
            isOnTop = height >= texelFetch(baseHeightField, texIndex).x;
        
        visibleTriangle = visibleTriangle ||
            (isOnTop &&
            normProjPos.x >= -1 && normProjPos.x <= 1
            && normProjPos.y >= -1 && normProjPos.y <= 1
            && normProjPos.z >= 0 && normProjPos.z <= 1);
    }
    
    if (!visibleTriangle)
        return;
    
    for (int i = 0; i < 3; ++i) {
        gl_Position = positions[i];
        EmitVertex();
    }
    EndPrimitive();
}
