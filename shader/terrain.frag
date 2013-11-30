#version 330


//partly from http://www.opengl.org/sdk/docs/tutorials/TyphoonLabs/Chapter_4.pdf

in vec3 normal;
in vec3 viewPos;
in vec3 worldPos;

uniform vec3 cameraposition;

//uniform vec3 lightdir;
//uniform vec3 lightdir2;
//uniform mat4 light;
//uniform mat4 light2;
//uniform vec4 lightambientglobal;
//uniform mat4 material;

vec3 lightdir = vec3(0.0, 6.5, 7.5);
vec3 lightdir2 = vec3(0.0, -8.0, 7.5);
vec4 lightambientglobal = vec4(0);

// some kind of sunlight..
mat4 light = mat4(vec4(0.0, 0.0, 0.0, 1.0),        //ambient
                  vec4(0.2, 0.2, 0.2, 1.0),        //diffuse
                  vec4(0.7, 0.7, 0.5, 1.0),        //specular
                  vec4(0.002, 0.002, 0.0004, 1.4)); //attenuation1, attenuation2, attenuation3, shininess

// zero for now
mat4 light2 = mat4(vec4(0.0, 0.0, 0.0, 1.0),        //ambient
                   vec4(0.0, 0.0, 0.0, 1.0),        //diffuse
                   vec4(0.0, 0.0, 0.0, 1.0),        //specular
                   vec4(0.002, 0.002, 0.0004, 1.4)); //attenuation1, attenuation2, attenuation3, shininess

// simple white terrain
mat4 material = mat4(vec4(0.0, 0.0, 0.0, 1.0),    //ambient
                     vec4(1.0, 1.0, 1.0, 1.0),    //diffuse
                     vec4(1.0, 1.0, 1.0, 1.0),    //specular
                     vec4(0, 0, 0, 0));            //emission

vec4 phongLighting(vec3 n, vec3 v_pos, vec3 cameraposition, vec3 lightdir, vec3 lightdir2, mat4 light, mat4 light2, vec4 lightambientglobal, mat4 material);

in vec2 screenPos;

uniform vec3 tileSize;

flat in float f_terrainTypeID;

layout(location = 0)out vec4 fragColor;

void main()
{
    vec4 lightColor = phongLighting(normal, viewPos, cameraposition, lightdir, lightdir2, light, light2, lightambientglobal, material);

    int terrainTypeID = int(f_terrainTypeID);

    vec3 terrainColor;

    switch (terrainTypeID) {
    case 0: discard;
        // terrainColor = vec3(1.0, 0.0, 0.0);
        // break;
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
