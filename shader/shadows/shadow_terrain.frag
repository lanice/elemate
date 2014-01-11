#version 330 core

uniform sampler2D lightMap;
uniform ivec2 viewport;

in vec4 v_shadowCoord;

layout(location = 0)out float shadowValue;

float zBias = 0.005;
float lightSize = 15.0f;
float searchWidth = 10.0f;

int nbSamples = 24;
uniform float shadowSamples[128];

float linearize(float depth);

float average_blocker_depth(vec2 coord, float zReceiver, float viewportsize) {
    float zSum = 0.0;
    int numBlockers = 0;
    for(int i=0; i < nbSamples; ++i) {
        float zBlocker = texture(lightMap, coord + shadowSamples[i] * searchWidth / viewportsize).z;
        if (zBlocker < zReceiver){
            zSum += zBlocker;
            ++numBlockers;
        } 
    }
    return numBlockers > 0 ? zSum/numBlockers : 0.0;
}

void main()
{
	vec4 coord = v_shadowCoord / v_shadowCoord.w;
    // float z = linearize(coord.z) - zBias;
    float z = coord.z - zBias;
    float viewportsize = float(viewport.x+viewport.y) * 0.5;

    float zBlocker = average_blocker_depth(coord.xy, z, viewportsize);

    if(zBlocker == 0.0) {
        shadowValue = 1.0;
        return;
    }
    
    float penumbra = (z - zBlocker) / zBlocker * lightSize / viewportsize;
    float shadow = 0.0;
    float x,y;
    for (int i=0; i < nbSamples; i++) { 
            vec2 offset = coord.st + shadowSamples[i] * penumbra;
            float distanceFromLight = texture(lightMap, offset).z;
            shadow += distanceFromLight <= z ? 0.0 : 1.0;
    }

    shadowValue = shadow / nbSamples;
}
