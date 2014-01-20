#version 330 core

in vec3 v_vertex[1];

uniform mat4 viewProjection;
uniform vec3 lookAtUp;
uniform vec3 lookAtRight;
uniform vec3 lookAtFront;
uniform float particleSize;


layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

out vec2 g_relPos;
flat out float g_zDiff;

void main()
{    
    vec4 projPos = viewProjection * vec4(v_vertex[0], 1.0);
    vec4 projPosRight = viewProjection * vec4(v_vertex[0] + lookAtRight * particleSize, 1.0);
    vec4 projPosUp = viewProjection * vec4(v_vertex[0] + lookAtUp * particleSize, 1.0);
    vec4 projPosFront = viewProjection * vec4(v_vertex[0] + lookAtFront * particleSize, 1.0);
    
	float projDiffx = distance(projPosRight.xyz, projPos.xyz);
	float projDiffy = distance(projPosUp.xyz, projPos.xyz);
    float projDiffz = distance(projPosFront.xyz, projPos.xyz);
	
    gl_Position = projPos + vec4(-projDiffx, -projDiffy, 0.0, 0.0);
	g_relPos = vec2(-1,-1);
    g_zDiff = projDiffz;
    EmitVertex();
    
    gl_Position = projPos + vec4(-projDiffx, projDiffy, 0.0, 0.0);
	g_relPos = vec2(-1,1);
    g_zDiff = projDiffz;
    EmitVertex();
    
    gl_Position = projPos + vec4(projDiffx, -projDiffy, 0.0, 0.0);
	g_relPos = vec2(1,-1);
    g_zDiff = projDiffz;
    EmitVertex();
    
    gl_Position = projPos + vec4(projDiffx, projDiffy, 0.0, 0.0);
	g_relPos = vec2(1,1);
    g_zDiff = projDiffz;
    EmitVertex();
    
    EndPrimitive();
}
