#version 330


//partly from http://www.opengl.org/sdk/docs/tutorials/TyphoonLabs/Chapter_4.pdf

in vec3 normal;
in vec3 position;
in vec3 worldPos;

uniform vec3 cameraposition;
//vec3 cameraposition = vec3(0.0, 10.0, 12.0);

//uniform vec3 lightdir;
//uniform vec3 lightdir2;
//uniform mat4 light;
//uniform mat4 light2;
//uniform vec4 lightambientglobal;
//uniform mat4 material;

vec3 lightdir = vec3(0.0, 6.5, 7.5);
vec3 lightdir2 = vec3(0.0, -8.0, 7.5);
vec4 lightambientglobal = vec4(0);

mat4 light = mat4(vec4(0.0, 0.0, 0.0, 1.0),        //ambient
                  vec4(0.2, 0.2, 0.2, 1.0),        //diffuse
                  vec4(0.0, 0.0, 0.8, 1.0),        //specular
                  vec4(0.002, 0.002, 0.0004, 1.4)); //attenuation1, attenuation2, attenuation3, shininess

mat4 light2 = mat4(vec4(0.0, 0.0, 0.0, 1.0),        //ambient
                   vec4(0.1, 0.1, 0.1, 1.0),        //diffuse
                   vec4(0.8, 0.0, 0.0, 1.0),        //specular
                   vec4(0.002, 0.002, 0.0004, 1.4)); //attenuation1, attenuation2, attenuation3, shininess

mat4 material = mat4(vec4(0.1, 0.1, 0.1, 1.0),    //ambient
                     vec4(1.0, 1.0, 1.0, 1.0),    //diffuse
                     vec4(1.0, 1.0, 1.0, 1.0),    //specular
                     vec4(0, 0, 0, 0));            //emission

vec4 phongLighting(vec3 n, vec3 v_pos, vec3 cameraposition, vec3 lightdir, vec3 lightdir2, mat4 light, mat4 light2, vec4 lightambientglobal, mat4 material);

uniform usampler2D terrainType;

uniform mat4 filterMatrix;

in vec2 screenPos;

uniform vec3 tileSize;

void main()
{
	vec3 n = normal;

    vec2 texCoord = worldPos.xy + (tileSize.xz * 0.5);
    texCoord = texCoord / tileSize.xz;

    unsigned int value = texture(terrainType, texCoord);

    vec3 color;

    switch (value)
    {
    case 0u: color = vec3(1.0, 0.0, 0.0);
        break;
    case 1u: color = vec3(0.0, 1.0, 0.0);
        break;
    case 2u: color = vec3(1.0, 0.0, 0.0);
        break;
    case 3u: color = vec3(1.0, 1.0, 0.0);
        break;
    default: color = vec3((float(value) / 2147483648.0));
        break;
    }
	
    //gl_FragColor = phongLighting(n, position, cameraposition, lightdir, lightdir2, light, light2, lightambientglobal, material);
    gl_FragColor = vec4(color, 1);
}
