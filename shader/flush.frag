#version 330 core

in vec2 v_uv;

uniform sampler2D sceneColor;
uniform sampler2D sceneDepth;
uniform sampler2D waterNormals;
uniform sampler2D waterDepth;

layout(location = 0)out vec4 fragColor;

void main()
{
    float sceneZ = texture(sceneDepth, v_uv).r;
    float waterZ = texture(waterDepth, v_uv).r;
	fragColor = mix(
		//vec4(0.2, 0.3, 0.8, 0.8),
		vec4(vec3(texture(waterDepth, v_uv).r), 1.0),
		texture(sceneColor, v_uv),
		step(sceneZ,waterZ)
	);
}
