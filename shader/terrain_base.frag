#version 330 core

in vec3 viewPos;
in int texIndex;

uniform vec3 cameraposition;

uniform vec4 lightambientglobal;
uniform vec3 lightdir1;
uniform vec3 lightdir2;

uniform mat4 light1;
uniform mat4 light2;

uniform mat4 material_bedrock;
uniform mat4 material_dirt;

uniform usamplerBuffer terrainTypeID;

vec4 phongLighting(vec3 n, vec3 v_pos, vec3 cameraposition, vec3 lightdir1, vec3 lightdir2, mat4 light1, mat4 light2, vec4 lightambientglobal, mat4 material);


layout(location = 0)out vec4 fragColor;

void main()
{
    vec3 normal = vec3(0, 1, 0);
    
    uint id = texelFetch(terrainTypeID, texIndex).x;
    
    switch(id) {
    case 0u:
        fragColor = vec4(1.0, 0.0, 0.0, 1.0);
        return;
    case 1u:
        fragColor = phongLighting(normal, viewPos, cameraposition, lightdir1, lightdir2, light1, light2, lightambientglobal, material_bedrock);
        return;
    case 2u:
        fragColor = phongLighting(normal, viewPos, cameraposition, lightdir1, lightdir2, light1, light2, lightambientglobal, material_dirt);
        return;
    default:
        discard;
    }
}
