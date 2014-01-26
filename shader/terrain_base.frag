#version 330 core

in vec3 g_worldPos;
in vec3 g_normal;
in vec3 g_viewPos;
flat in int g_texIndex;

uniform vec3 lightdir2;

uniform mat4 light2;

uniform mat4 element_bedrock;
uniform mat4 element_dirt;

uniform usamplerBuffer terrainTypeID;
uniform sampler2D rockSampler;

vec4 phongLighting(vec3 n, vec3 v_pos, vec3 lightdir2, mat4 light2, mat4 material);


layout(location = 0)out vec4 fragColor;

void main()
{
    uint id = texelFetch(terrainTypeID, g_texIndex).x;
    
    vec4 textureColor = texture(rockSampler, g_worldPos.xz * 0.1);
    vec4 lightColor = vec4(1.0, 0.0, 0.0, 1.0); // just to check that the terrainTypeID texture contains valid data
    
    switch(id) {
    case 0u:
        lightColor = phongLighting(g_normal, g_viewPos, lightdir2, light2, element_bedrock);
        break;
    case 1u:
        lightColor = phongLighting(g_normal, g_viewPos, lightdir2, light2, element_dirt);
        break;
    }
    
    fragColor = mix(lightColor, textureColor, 0.5);
}
