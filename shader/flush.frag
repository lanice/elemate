#version 330 core

in vec2 v_uv;

uniform sampler2D sceneColor;
uniform sampler2D sceneDepth;
uniform sampler2D handColor;
uniform sampler2D handDepth;
// uniform sampler2D particleNormals;
uniform sampler2D particleDepth;
uniform sampler2D shadowMap;
// uniform sampler2D lightMap;
uniform sampler2D rainSampler;
uniform usampler2D elementID;
uniform vec3 skyColor;
uniform samplerCube skybox;
uniform mat4 viewProjectionInverse;
uniform vec3 cameraPosition;
uniform int timef;
uniform float rainStrength;
uniform float humidityFactor;

layout(location = 0)out vec4 fragColor;

float linearize(float depth);
vec4 waterColor(vec2 v_uv, float worldHeight);
vec4 lavaColor(vec2 v_uv);
vec4 steamColor(vec2 v_uv);
vec4 sandColor(vec2 v_uv);

float shadowFactorLambert(float depth)
{
    float depthX1 = linearize(texture(sceneDepth, v_uv+vec2(0.004, 0.0)).r)-depth;
    float depthX2 = depth-linearize(texture(sceneDepth, v_uv+vec2(-0.004, 0.0)).r);
    float depthY1 = depth-linearize(texture(sceneDepth, v_uv+vec2(0.0, 0.004)).r);
    float depthY2 = linearize(texture(sceneDepth, v_uv+vec2(0.0, -0.004)).r)-depth;

    float depthX = mix(depthX2,depthX1,step(abs(depthX1),abs(depthX2)));
    float depthY = mix(depthY2,depthY1,step(abs(depthY1),abs(depthY2)));

    if (abs(depthX)>0.004 || abs(depthY)>0.004)
        return 0.6;

    vec3 vecX = vec3(0.004, 0.0, depthX);
    vec3 vecY = vec3(0.0, -0.004, depthY);

    vec3 n = normalize(cross(vecX,vecY));
    return 0.7*pow(dot(n,vec3(0.0, 0,-1.0)),2)+0.3;
}

void main()
{
    float sceneZ = texture(sceneDepth, v_uv).r;
    float linearSceneZ = linearize(sceneZ);
    float handZ = texture(handDepth, v_uv).r;
    float linearParticleZ = texture(particleDepth, v_uv).r;
    vec4 sceneC = texture(sceneColor, v_uv);
    vec4 handC = texture(handColor, v_uv);
    float shadowFactor = texture(shadowMap, v_uv).x * 0.7 + 0.3;
    vec4 sceneHandColor = mix(
        texture(handColor, v_uv),
        texture(sceneColor, v_uv)*shadowFactorLambert(linearSceneZ),
        step(sceneZ, handZ));
    float sceneHandZ = min(sceneZ, handZ);
    float linearSceneHandZ = linearize(sceneHandZ);
    float rainDepth = 1.0-texture(rainSampler, vec2(mod(timef/5000,100)/22.0+v_uv.x/11.0,v_uv.y)).r;

    uint element = texture(elementID, v_uv).x;
    
    // overall depth of the current fragment
    float linearFragZ = min(linearSceneHandZ, linearParticleZ);
    
    // world position and view direction to the current fragment
    vec4 fragPosWorld4 = (viewProjectionInverse * vec4(v_uv * 2.0 - 1.0, 0, 1.0));
    vec3 fragPosWorld = fragPosWorld4.xyz / fragPosWorld4.w;
    vec3 fragViewDirWorld = fragPosWorld - cameraPosition;
    
    // height of the solid terrain/hand fragment in world coordinates
    vec4 scenePosWorld4 = (viewProjectionInverse * vec4(v_uv * 2.0 - 1.0, sceneHandZ, 1.0));
    float worldHeight = scenePosWorld4.y / scenePosWorld4.w;

    
    vec4 particleC;     // mapping defined in particledrawable.cpp for now
    switch (element) {
    case 1u:
        particleC = waterColor(v_uv, worldHeight);
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
    case 5u:
        particleC = steamColor(v_uv);
        break;
    default:
        particleC = vec4(1,1,1,1);
    }
        
    const vec3 fogColor = vec3(0.4);
    
    vec3 fragColorRgbScene = mix( // mix the scene/element color (with shadow) with the horizon/fog blending
        mix(
            (1-particleC.w * (1-shadowFactor)) * particleC.rgb,
            shadowFactor * sceneHandColor.rgb,
            step(linearSceneHandZ, linearParticleZ)),
        fogColor,
        clamp(linearFragZ * linearFragZ + humidityFactor, 0, 1)
    );
    
    // use the skybox where needed
    vec3 fragColorRgbSky = mix(
        fogColor,
        texture(skybox, fragViewDirWorld + vec3(0, 0.01, 0)).rgb,
        smoothstep(-0.05, -0.02, fragViewDirWorld.y));
    
    vec3 fragColorRgb = mix(fragColorRgbScene, fragColorRgbSky, step(1.0, linearFragZ));
    
    // let it rain
    fragColorRgb = mix(
        fragColorRgb,
        vec3(0.5, 0.5, 0.5),
        step(rainDepth, linearFragZ) * rainStrength * (1.0-rainDepth) * (1.0-rainDepth)
    );

    fragColor = vec4(fragColorRgb, 1.0);
}