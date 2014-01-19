#version 330 core

uniform sampler2D lightMap;
uniform ivec2 viewport;

in vec4 v_shadowCoord;

layout(location = 0)out float shadowValue;

float zBias = 0.005;
float lightSize = 15.0f;
float searchWidth = 10.0f;

int nbSamples = 32;
uniform vec2 depthSamples[32];

float linearize(float depth);

float average_blocker_depth(vec2 coord, float zReceiver, float viewportsize) {
    float zSum = 0.0;
    int numBlockers = 0;
    for(int i=0; i < nbSamples; ++i) {
        float zBlocker = texture(lightMap, coord + depthSamples[i] * searchWidth / viewportsize).x;
        if (zBlocker < zReceiver){
            zSum += zBlocker;
            ++numBlockers;
        } 
    }
    return numBlockers > 0 ? zSum/numBlockers : 0.0;
}

void main()
{
	vec4 fromLightCoord = v_shadowCoord / v_shadowCoord.w;
    // float fromLightZ = linearize(fromLightCoord.z) - zBias;
    float fromLightZ = fromLightCoord.z - zBias;
    float viewportsize = float(viewport.x+viewport.y) * 0.5;

    float zBlocker = average_blocker_depth(fromLightCoord.xy, fromLightZ, viewportsize);
    
    if(zBlocker == 0.0) {
        shadowValue = 1.0;
        return;
    }
    
    float penumbra = (fromLightZ - zBlocker) / zBlocker * lightSize / viewportsize;
    float shadow = 0.0;
    float x,y;
    for (int i=0; i < nbSamples; i++) { 
            vec2 offset = fromLightCoord.st + depthSamples[i] * penumbra;
            float distanceFromLight = texture(lightMap, offset).x;
            shadow += distanceFromLight <= fromLightZ ? 0.0 : 1.0;
    }

    shadowValue = shadow / nbSamples;
}
