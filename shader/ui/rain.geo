#version 330 core

in vec3 v_vertex[1];

uniform mat4 projection;

layout (points) in;
layout (triangle_strip​, max_vertices = 4) out;

void main()
{
    vec3 Pos = v_vertex[0];

    gl_Position = projection * vec4(-0.5,-0.5,0.5,1.0);//vec4(Pos - vec3(-0.05, -0.05, 0.0),1.0);
    EmitVertex();

    gl_Position = projection * vec4(-0.5,0.5,0.5,1.0);//vec4(Pos - vec3(-0.05, 0.05, 0.0),1.0);
    EmitVertex();

    gl_Position = projection * vec4(0.5,0.5,0.5,1.0);//vec4(Pos - vec3(0.05, 0.05, 0.0),1.0);
    EmitVertex();

    gl_Position = projection * vec4(0.5,-0.5,0.5,1.0);//vec4(Pos - vec3(0.05, -0.05, 0.0),1.0);
    EmitVertex();

    EndPrimitive();
}
