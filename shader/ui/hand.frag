#version 330 core

in vec3 v_normal;
in vec3 v_viewPos;

uniform vec3 lightdir2;

uniform mat4 light2;

layout(location = 0)out vec4 fragColor;

vec4 phongLighting(vec3 n, vec3 v_pos, vec3 lightdir2, mat4 light2, mat4 material);

mat4 material = mat4(vec4(0.0, 0.0, 0.0, 1.0),    //ambient
                     vec4(0.8, 0.6, 0.6, 1.0),    //diffuse
                     vec4(0.8, 0.6, 0.6, 1.0),    //specular
                     vec4(0, 0, 0, 0));            //emission

void main()
{
    fragColor = phongLighting(v_normal, v_viewPos, lightdir2, light2, material);
}
