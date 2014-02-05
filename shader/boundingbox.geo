#version 330 core

in vec3 v_vertex[2]; // llf, urb

layout (lines) in;
layout (triangle_strip, max_vertices = 6) out;

uniform mat4 MVP;

void main()
{
    vec3 llf = v_vertex[0];
    vec3 urb = v_vertex[1];

    gl_Position = MVP * vec4(llf.x, urb.y, llf.z, 1.0);
    EmitVertex();
    gl_Position = MVP * vec4(llf.x, urb.yz, 1.0);
    EmitVertex();
    gl_Position = MVP * vec4(urb.xy, llf.z, 1.0);
    EmitVertex();
    gl_Position = MVP * vec4(urb.xyz, 1.0);
    EmitVertex();
    EndPrimitive();
}
