#version 330

// lightning partly from http://www.opengl.org/sdk/docs/tutorials/TyphoonLabs/Chapter_4.pdf
//      ... and https://github.com/hpicgs/cgsee

in vec3 normal;
in vec3 viewPos;

uniform vec3 cameraposition;

uniform vec4 lightambientglobal;
uniform vec3 lightdir1;
uniform vec3 lightdir2;

uniform mat4 light1;
uniform mat4 light2;

uniform mat4 material_water;

vec4 phongLighting(vec3 n, vec3 v_pos, vec3 cameraposition, vec3 lightdir1, vec3 lightdir2, mat4 light1, mat4 light2, vec4 lightambientglobal, mat4 material);

layout(location = 0)out vec4 fragColor;

void main()
{    
    fragColor = vec4(
        phongLighting(normal, viewPos, cameraposition, lightdir1, lightdir2, light1, light2, lightambientglobal,
            material_water).rgb,
        0.5);
}