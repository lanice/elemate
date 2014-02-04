#version 330 core

in vec2 v_uv;

uniform sampler2D sceneColor;
uniform sampler2D sceneDepth;
uniform sampler2D handColor;
uniform sampler2D handDepth;
uniform sampler2D waterNormals;
uniform sampler2D waterDepth;
uniform sampler2D shadowMap;
uniform sampler2D lightMap;

layout(location = 0)out vec4 fragColor;

float linearize(float depth);
vec4 waterColor(vec2 v_uv);
vec4 lavaColor(vec2 v_uv);

void main()
{
    fragColor = vec4(texture(shadowMap, v_uv).xxx, 1.0);
    // fragColor = vec4(texture(lightMap, v_uv).xxx, 1.0);
    // fragColor = vec4(texture(waterDepth, v_uv).xxx, 1.0);
    // return;
    // fragColor = texture(sceneColor, v_uv);
    // return;
    float sceneZ = linearize(texture(sceneDepth, v_uv).r);
    float handZ = linearize(texture(handDepth, v_uv).r);
    float waterZ = texture(waterDepth, v_uv).r;
    vec4 sceneC = texture(sceneColor, v_uv);
    vec4 handC = texture(handColor, v_uv);
    vec4 waterC = waterColor(v_uv);
    vec4 lavaC = lavaColor(v_uv);
    float shadowFactor = texture(shadowMap, v_uv).x * 0.7 + 0.3;

    fragColor = vec4(handZ);
    // return;
    vec4 sceneHandColor;
    if (sceneZ < handZ)
        sceneHandColor = sceneC;
    else
        sceneHandColor = handC;
    float sceneHandZ = min(sceneZ, handZ);

	fragColor = 
    
    mix(
        (1-lavaC.w * (1-shadowFactor)) * lavaC,
		shadowFactor * sceneHandColor,
		step(sceneHandZ,waterZ)
	);
}