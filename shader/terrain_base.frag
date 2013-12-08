#version 330

// lightning partly from http://www.opengl.org/sdk/docs/tutorials/TyphoonLabs/Chapter_4.pdf
//      ... and https://github.com/hpicgs/cgsee

in vec3 normal;
in vec3 viewPos;
in vec3 worldPos;
in vec2 screenPos;
flat in ivec2 terrainIdTexCoord;
in vec2 normInTileCoord;

uniform sampler2D terrainIDSampler;

uniform sampler2D rockSampler;

uniform vec3 cameraposition;

uniform vec4 lightambientglobal;
uniform vec3 lightdir1;
uniform vec3 lightdir2;

uniform mat4 light1;
uniform mat4 light2;

// simple white terrain
uniform mat4 material_default;
uniform mat4 material_bedrock;
uniform mat4 material_dirt;
uniform mat4 material_grassland;

vec4 phongLighting(vec3 n, vec3 v_pos, vec3 cameraposition, vec3 lightdir1, vec3 lightdir2, mat4 light1, mat4 light2, vec4 lightambientglobal, mat4 material);

layout(location = 0)out vec4 fragColor;

void main()
{
    float f_terrainTypeID = texelFetch(terrainIDSampler, terrainIdTexCoord, 0).r;
    int terrainTypeID = int(f_terrainTypeID * 3);
    
    vec4 textureColor = texture(rockSampler, normInTileCoord * 10);
    
    vec4 lightColor;
    
    switch (terrainTypeID) {
    case 0: 
        lightColor =  
            phongLighting(normal, viewPos, cameraposition, lightdir1, lightdir2, light1, light2, lightambientglobal,
            material_bedrock);
        break;
    case 2: 
        lightColor =  
            phongLighting(normal, viewPos, cameraposition, lightdir1, lightdir2, light1, light2, lightambientglobal,
            material_dirt);
        break;
    }
    
    fragColor = mix(lightColor, textureColor, 0.7);
}
