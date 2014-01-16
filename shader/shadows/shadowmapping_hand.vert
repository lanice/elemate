#version 330 core

layout(location = 0)in vec3 _vertex;

uniform mat4 modelViewProjection;

uniform mat4 lightBiasMVP;

out vec4 v_shadowCoord;

void main()
{    
    gl_Position = modelViewProjection * vec4(_vertex, 1.0);
    
    v_shadowCoord = lightBiasMVP * vec4(_vertex, 1.0);
}
