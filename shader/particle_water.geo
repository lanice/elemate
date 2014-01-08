#version 330 core

in vec3 v_vertex[1];

uniform mat4 viewProjection;
uniform vec3 lookAtUp;
uniform vec3 lookAtRight;


layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

out vec2 g_relPos;
out vec4 g_absPos;
out vec2 factor;

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
	g_relPos = vec2(-1,-1);
    g_absPos = gl_Position;
    factor = vec2(factorX, factorY);
    EmitVertex();
    
    gl_Position = projPos + vec4(-factorX, factorY, 0.0, 0.0);
	g_relPos = vec2(-1,1);
    g_absPos = gl_Position;
    factor = vec2(factorX, factorY);
    EmitVertex();
    
    gl_Position = projPos + vec4(factorX, -factorY, 0.0, 0.0);
	g_relPos = vec2(1,-1);
    g_absPos = gl_Position;
    factor = vec2(factorX, factorY);
    EmitVertex();
    
    gl_Position = projPos + vec4(factorX, factorY, 0.0, 0.0);
	g_relPos = vec2(1,1);
    g_absPos = gl_Position;
    factor = vec2(factorX, factorY);
    EmitVertex();
    
    EndPrimitive();
}
