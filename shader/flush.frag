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
uniform int timef;

layout(location = 0)out vec4 fragColor;

float linearize(float depth);
vec4 waterColor(vec2 v_uv);
vec4 lavaColor(vec2 v_uv);
vec4 steamColor(vec2 v_uv);
vec4 sandColor(vec2 v_uv);

float shadowFactorLambert(float depth){
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
    float sceneZ = linearize(texture(sceneDepth, v_uv).r);
    float handZ = linearize(texture(handDepth, v_uv).r);
    float particleZ = texture(particleDepth, v_uv).r;
    vec4 sceneC = texture(sceneColor, v_uv);
    vec4 handC = texture(handColor, v_uv);
    float shadowFactor = texture(shadowMap, v_uv).x * 0.7 + 0.3;
    vec4 sceneHandColor = mix(texture(handColor, v_uv), texture(sceneColor, v_uv)*shadowFactorLambert(sceneZ), step(sceneZ, handZ));
    float sceneHandZ = min(sceneZ, handZ);
    float rainDepth = 1.0-texture(rainSampler, vec2(mod(timef/1000,16)/16.0+v_uv.x/8.0,v_uv.y)).r;

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
    case 5u:
        particleC = steamColor(v_uv);
        break;
    default:
        particleC = vec4(1,1,1,1);
    }
    
    vec3 sceneColor =  mix(
            (1-particleC.w * (1-shadowFactor)) * particleC.rgb,
            shadowFactor * sceneHandColor.rgb,
            step(sceneHandZ,particleZ)
        );
    vec3 fragColorRgb = mix(
        mix(vec3(0.5,0.5,0.5),sceneColor,rainDepth*0.8+0.2),
        sceneColor,
        step(min(sceneHandZ,particleZ),rainDepth)
    );
    
    float fragZ = min(sceneHandZ, particleZ);
    // blend (mostly) at the horizon
    fragColor = vec4(mix(fragColorRgb, skyColor, fragZ * fragZ), 1.0);
}