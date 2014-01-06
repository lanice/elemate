#version 330 core

in vec3 v_vertex[1];

uniform mat4 viewProjection;
uniform vec3 lookAtUp;
uniform vec3 lookAtRight;


layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

void main()
{
    float size = 0.1;
    
    vec4 projPos = viewProjection * vec4(v_vertex[0], 1.0);
    
    vec4 pos2 = viewProjection * vec4(v_vertex[0] + lookAtRight, 1.0);
	float diffx = distance(pos2.xyz, projPos.xyz);
	pos2 = viewProjection * vec4(v_vertex[0] + lookAtUp, 1.0);
	float diffy = distance(pos2.xyz, projPos.xyz);
    
    float factorX = size * diffx;
	float factorY = size * diffy;

    gl_Position = projPos + vec4(-factorX, -factorY, 0.0, 0.0);
    EmitVertex();
    
    gl_Position = projPos + vec4(-factorX, factorY, 0.0, 0.0);
    EmitVertex();
    
    gl_Position = projPos + vec4(factorX, -factorY, 0.0, 0.0);
    EmitVertex();
    
    gl_Position = projPos + vec4(factorX, factorY, 0.0, 0.0);
    EmitVertex();
    
    EndPrimitive();
}