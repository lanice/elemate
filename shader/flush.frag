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
vec4 sandColor(vec2 v_uv);



uniform int timef;

vec4 rain() {
    return vec4(fract(sin(timef*dot(vec2(v_uv.x,floor(v_uv.y*20)/20.0) ,vec2(12.9898,78.233))) * 43758.5453));
}

void main()
{    
    float sceneZ = linearize(texture(sceneDepth, v_uv).r);
    float handZ = linearize(texture(handDepth, v_uv).r);
    float particleZ = texture(particleDepth, v_uv).r;
    vec4 sceneC = texture(sceneColor, v_uv);
    vec4 handC = texture(handColor, v_uv);
    float shadowFactor = texture(shadowMap, v_uv).x * 0.7 + 0.3;

    vec4 sceneHandColor = mix(texture(handColor, v_uv), texture(sceneColor, v_uv), step(sceneZ, handZ));
    float sceneHandZ = min(sceneZ, handZ);

    uint element = texture(elementID, v_uv).x;
    
    vec4 particleC;     // mapping defined in particledrawable.cpp for now
    switch (element) {
    case 1u:
        particleC = waterColor(v_uv);
        break;
    case 2u:
        particleC = lavaColor(v_uv);
        break;
    case 3u:
        particleC = sandColor(v_uv);
        break;
    case 4u:
        particleC = vec4(0.3, 0.3, 0.3, 1.0);
        break;
    default:
        particleC = vec4(1,1,1,1);
    }
    
	fragColor =
    mix(
        (1-particleC.w * (1-shadowFactor)) * particleC,
		shadowFactor * sceneHandColor,
		step(sceneHandZ,particleZ)
	);
    fragColor = rain();
}