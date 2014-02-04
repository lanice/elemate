#version 330 core

in vec2 v_uv;

uniform sampler2D sceneColor;
uniform sampler2D sceneDepth;
uniform sampler2D handColor;
uniform sampler2D handDepth;
uniform sampler2D particleNormals;
uniform sampler2D particleDepth;
uniform sampler2D shadowMap;
uniform sampler2D lightMap;
uniform usampler2D elementID;

layout(location = 0)out vec4 fragColor;

float linearize(float depth);
vec4 waterColor(vec2 v_uv);
vec4 lavaColor(vec2 v_uv);

void main()
{    
    fragColor = vec4(float(texture(elementID, v_uv)) * 0.25);
    return;
    
    float sceneZ = linearize(texture(sceneDepth, v_uv).r);
    float handZ = linearize(texture(handDepth, v_uv).r);
    float particleZ = texture(particleDepth, v_uv).r;
    vec4 sceneC = texture(sceneColor, v_uv);
    vec4 handC = texture(handColor, v_uv);
    vec4 waterC = waterColor(v_uv);
    vec4 lavaC = lavaColor(v_uv);
    float shadowFactor = texture(shadowMap, v_uv).x * 0.7 + 0.3;

    vec4 sceneHandColor = mix(texture(handColor, v_uv), texture(sceneColor, v_uv), step(sceneZ, handZ));
    float sceneHandZ = min(sceneZ, handZ);

	fragColor =
    mix(
        (1-lavaC.w * (1-shadowFactor)) * lavaC,
		shadowFactor * sceneHandColor,
		step(sceneHandZ,particleZ)
	);
}