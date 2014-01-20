#version 330 core

in vec3 v_vertex[1];

uniform mat4 viewProjection;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 lookAtUp;
uniform vec3 lookAtRight;
uniform vec3 lookAtFront;
uniform float particleSize;


layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

out vec2 g_relPos;
out vec4 g_viewPos;
flat out vec4 g_viewPosCenter;
// flat out float g_zDiff;


vec4 dRightUp = vec4(particleSize, particleSize, 0.0, 0.0);

void addVertex(vec4 viewPosCenter, vec2 relPos)
{
	g_relPos = relPos;
    g_viewPosCenter = viewPosCenter;
    g_viewPos = viewPosCenter + dRightUp * vec4(relPos, 0.0, 0.0);
    gl_Position = projection * g_viewPos;
    EmitVertex();
}

void main()
{    
    vec4 viewPosCenter = view * vec4(v_vertex[0], 1.0);
	
	addVertex(viewPosCenter, vec2(-1,-1));
	addVertex(viewPosCenter, vec2(-1, 1));
	addVertex(viewPosCenter, vec2( 1,-1));
	addVertex(viewPosCenter, vec2( 1, 1));
    
    EndPrimitive();
}
