#version 330 core

in vec3 v_vertex[2]; // llf, urb

layout (lines) in;
layout (line_strip, max_vertices = 16) out;

uniform mat4 MVP;

void emitVertex(vec3 vertex) {
    gl_Position = MVP * vec4(vertex, 1.0);
    EmitVertex();
}

void main()
{
    vec3 llf = v_vertex[0];
    vec3 urb = v_vertex[1];
    
    emitVertex(llf);
    emitVertex(vec3(urb.x, llf.yz));
    emitVertex(vec3(urb.xy, llf.z));
    emitVertex(vec3(llf.x, urb.y, llf.z));
    emitVertex(llf);
    emitVertex(vec3(llf.xy, urb.z));
    emitVertex(vec3(urb.x, llf.y, urb.z));
    emitVertex(urb);
    emitVertex(vec3(llf.x, urb.yz));
    emitVertex(vec3(llf.xy, urb.z));
    EndPrimitive();
    emitVertex(vec3(llf.x, urb.y, llf.z));
    emitVertex(vec3(llf.x, urb.yz));
    EndPrimitive();
    emitVertex(vec3(urb.xy, llf.z));
    emitVertex(vec3(urb.xyz));
    EndPrimitive();
    emitVertex(vec3(urb.x, llf.yz));
    emitVertex(vec3(urb.x, llf.y, urb.z));
    EndPrimitive();
}
