#version 330 core

in vec2 v_uv;

uniform sampler2D sceneColor;
uniform sampler2D sceneDepth;
uniform sampler2D waterNormals;
uniform sampler2D waterDepth;
uniform sampler2D shadowMap;
uniform sampler2D lightMap;

layout(location = 0)out vec4 fragColor;

void main()
{
    // fragColor = vec4(texture(shadowMap, v_uv).xxx, 1.0);
    // fragColor = vec4(texture(lightMap, v_uv).xxx, 1.0);
    // return;
    // fragColor = texture(sceneColor, v_uv);
    // return;
    float sceneZ = texture(sceneDepth, v_uv).r;
    float waterZ = texture(waterDepth, v_uv).r;
	fragColor = 
    texture(shadowMap, v_uv).x * 
    mix(
		vec4(0.2, 0.3, 0.8, 0.8),
		texture(sceneColor, v_uv),
		step(sceneZ,waterZ)
	);
}
