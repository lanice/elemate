#version 330

in vec3 normal;
in vec3 viewPos;

uniform vec3 cameraposition;

uniform vec4 lightambientglobal;
uniform vec3 lightdir1;
uniform vec3 lightdir2;

uniform mat4 light1;
uniform mat4 light2;

mat4 material = mat4(vec4(0.0, 0.0, 0.0, 1.0),    //ambient
                     vec4(0.8, 0.6, 0.6, 1.0),    //diffuse
                     vec4(0.8, 0.6, 0.6, 1.0),    //specular
                     vec4(0, 0, 0, 0));            //emission

vec4 phongLighting(vec3 n, vec3 v_pos, vec3 cameraposition, vec3 lightdir1, vec3 lightdir2, mat4 light1, mat4 light2, vec4 lightambientglobal, mat4 material);

layout(location = 0)out vec4 fragColor;

void main()
{
    fragColor = phongLighting(normal, viewPos, cameraposition, lightdir1, lightdir2, light1, light2, lightambientglobal, material);
}