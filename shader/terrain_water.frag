#version 330 core

in vec3 v_viewPos;
in vec3 v_normal;
in float v_heightDiff;

uniform vec3 cameraposition;

uniform vec4 lightambientglobal;
uniform vec3 sunlightInvDir;
uniform vec3 lightdir2;

uniform mat4 sunlighting;
uniform mat4 light2;

uniform mat4 material_water;

vec4 phongLighting(vec3 n, vec3 v_pos, vec3 cameraposition, vec3 lightdir1, vec3 lightdir2, mat4 light1, mat4 light2, vec4 lightambientglobal, mat4 material);


layout(location = 0)out vec4 fragColor;

void main()
{ 
	if (v_heightDiff < 0)
		discard;
	
    fragColor = vec4(
        phongLighting(v_normal, v_viewPos, cameraposition, sunlightInvDir, lightdir2, sunlighting, light2, lightambientglobal, material_water).rgb,
        0.5);
}
