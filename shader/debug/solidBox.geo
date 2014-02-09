#version 330 core

in vec3 v_vertex[2]; // llf, urb

layout (lines) in;
layout (triangle_strip, max_vertices = 18) out;

uniform mat4 MVP;

void emitVertex(vec3 vertex) {
    gl_Position = MVP * vec4(vertex, 1.0);
    EmitVertex();
}

void main()
{
    vec3 llf = v_vertex[0];
    vec3 urb = v_vertex[1];
    
    // draw a cuboid:
    // front
    emitVertex(llf);
    emitVertex(vec3(llf.x, urb.y, llf.z));
    emitVertex(vec3(urb.x, llf.yz));
    emitVertex(vec3(urb.xy, llf.z));
    // right
    emitVertex(vec3(urb.x, llf.y, urb.z));
    emitVertex(vec3(urb.x, urb.yz));
    // back
    emitVertex(vec3(llf.xy, urb.z));
    emitVertex(vec3(llf.x, urb.y, urb.z));
    // left
    emitVertex(llf);
    emitVertex(vec3(llf.x, urb.y, llf.z));
    EndPrimitive();
    // top
    emitVertex(vec3(llf.x, urb.y, llf.z));
    emitVertex(vec3(llf.x, urb.yz));
    emitVertex(vec3(urb.xy, llf.z));
    emitVertex(urb);
    EndPrimitive();
    // bottom
    emitVertex(llf);
    emitVertex(vec3(urb.x, llf.yz));
    emitVertex(vec3(llf.xy, urb.z));
    emitVertex(urb);
    EndPrimitive();
}
