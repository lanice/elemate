#version 330

in vec3 v_normal[3];
in vec3 v_viewPos[3];
in vec3 v_worldPos[3];
in vec4 v_projPos[3];

out vec3 normal;
out vec3 viewPos;

layout (triangle_strip, max_vertices = 3) out;

void main()
{    
    for (int i=0; i < 3; ++i) {    
        normal = v_normal[i];
        viewPos = v_viewPos[i];
        gl_Position = v_projPos[i];
        EmitVertex();
    }
        
    EndPrimitive();
}
