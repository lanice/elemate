#version 330 core

in vec3 g_worldPos;
in vec3 g_normal;
in vec3 g_viewPos;
flat in int g_texIndex;

uniform float zfar;

uniform vec3 lightdir2;
uniform mat4 light2;
uniform vec3 skyColor;

uniform mat4 element_bedrock;
uniform mat4 element_sand;
uniform mat4 element_grassland;

uniform usamplerBuffer terrainTypeID;
uniform sampler2D bedrockSampler;
uniform sampler2D sandSampler;
uniform sampler2D grasslandSampler;

vec4 phongLighting(vec3 n, vec3 v_pos, vec3 lightdir2, mat4 light2, mat4 material);


layout(location = 0)out vec4 fragColor;

void main()
{
    uint id = texelFetch(terrainTypeID, g_texIndex).x;
    
    vec4 bedrockTex = texture(bedrockSampler, g_worldPos.xz * 0.1);
    vec4 sandTex = texture(sandSampler, g_worldPos.xz * 0.3);
    vec4 grasslandTex = texture(grasslandSampler, g_worldPos.xz * 0.3);

    vec4 lightColor = vec4(1.0, 0.0, 0.0, 1.0); // just to check that the terrainTypeID texture contains valid data
    vec4 textureColor;
    
    switch(id) {
    case 0u:
        lightColor = phongLighting(g_normal, g_viewPos, lightdir2, light2, element_bedrock);
        textureColor = bedrockTex;
        break;
    case 1u:
        lightColor = phongLighting(g_normal, g_viewPos, lightdir2, light2, element_sand);
        textureColor = sandTex;
        break;
    case 2u:
        lightColor = phongLighting(g_normal, g_viewPos, lightdir2, light2, element_grassland);
        textureColor = grasslandTex;
        break;
    }
    
    fragColor = mix(lightColor, textureColor, 0.5);
    fragColor = mix(fragColor, vec4(skyColor, 1.0), // blend at the horizon 
        max((gl_FragCoord.z / gl_FragCoord.w - (zfar * 0.9)) / (zfar * 0.1), 0.0));
}
