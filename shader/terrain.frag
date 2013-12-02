#version 330

// lightning partly from http://www.opengl.org/sdk/docs/tutorials/TyphoonLabs/Chapter_4.pdf
//      ... and https://github.com/hpicgs/cgsee

in vec3 normal;
in vec3 viewPos;
in vec3 worldPos;
in vec2 screenPos;
flat in ivec2 texCoord;

uniform sampler2D terrainTypeIDs;

uniform vec3 cameraposition;

uniform vec4 lightambientglobal;
uniform vec3 lightdir1;
uniform vec3 lightdir2;

uniform mat4 light1;
uniform mat4 light2;

// simple white terrain
mat4 material = mat4(vec4(0.0, 0.0, 0.0, 1.0),    //ambient
                     vec4(1.0, 1.0, 1.0, 1.0),    //diffuse
                     vec4(1.0, 1.0, 1.0, 1.0),    //specular
                     vec4(0, 0, 0, 0));            //emission

vec4 phongLighting(vec3 n, vec3 v_pos, vec3 cameraposition, vec3 lightdir1, vec3 lightdir2, mat4 light1, mat4 light2, vec4 lightambientglobal, mat4 material);

layout(location = 0)out vec4 fragColor;

void main()
{
    vec4 lightColor = phongLighting(normal, viewPos, cameraposition, lightdir1, lightdir2, light1, light2, lightambientglobal, material);

    float f_terrainTypeID = texelFetch(terrainTypeIDs, texCoord, 0).r;
    int terrainTypeID = int(f_terrainTypeID * 3); // terrain ids normalized to 0..1, get corresponding integer value (currently 3 terrain types, for testing)
    
    vec3 terrainColor;

    switch (terrainTypeID) {
    case 0: discard;
        /* terrainColor = vec3(1.0, 0.0, 0.0);
        break; */
    case 1: terrainColor = vec3(0.0, 1.0, 0.0);
        break;
    case 2: terrainColor = vec3(0.0, 0.0, 1.0);
        break;
    case 3: terrainColor = vec3(0.3, 0.7, 0.3);
        break;
    default: terrainColor = vec3(0.0, 0.0, 0.0);
    }
    fragColor = mix(vec4(terrainColor, 1.0),
        lightColor,
        0.5);
}
