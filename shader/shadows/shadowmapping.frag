#version 330 core

uniform sampler2D lightMap;
uniform float invViewportSize;

in vec4 v_shadowCoord;

layout(location = 0)out float shadowValue;

const float zBias = 0.003;
const float lightSize = 70.0f;
const float searchWidth = 5.0f;

const int nbSamples = 32;
uniform vec2 depthSamples[nbSamples];
const int nbEarlySamples = 5;
uniform vec2 earlyBailSamples[nbEarlySamples];

float average_blocker_depth(vec2 coord, float zReceiver, float relativeSearchWidth) {
    float zSum = 0.0;
    int numBlockers = 0;
    for(int i=0; i < nbSamples; ++i) {
        float zBlocker = texture(lightMap, depthSamples[i] * relativeSearchWidth + coord).x;
        if (zBlocker < zReceiver){
            zSum += zBlocker;
            ++numBlockers;
        } 
    }
    return numBlockers > 0 ? zSum/numBlockers : 0.0;
}

bool earlyBailing(vec2 coord, float zReceiver, float relativeSearchWidth);

void main()
{
    vec2 fromLightCoord = v_shadowCoord.st; // orthographic projection -> xyz/w not needed
    float fromLightZ = v_shadowCoord.z - zBias; // orthographic projection -> no depth linearisation
    
    float relativeSearchWidth = searchWidth * invViewportSize;

    if (earlyBailing(fromLightCoord, fromLightZ, relativeSearchWidth)) {
        return;
    }
    
    float zBlocker = average_blocker_depth(fromLightCoord, fromLightZ, relativeSearchWidth);
    
    if(zBlocker == 0.0) {
        shadowValue = 1.0;
        return;
    }
    
    float penumbra = (fromLightZ - zBlocker) / zBlocker * lightSize * invViewportSize;
    float shadow = 0.0;
    float x,y;
    for (int i=0; i < nbSamples; ++i) { 
            vec2 offset = depthSamples[i] * penumbra + fromLightCoord;
            float distanceFromLight = texture(lightMap, offset).x;
            shadow += mix( 1.0, 0.0, step(distanceFromLight, fromLightZ));
    }

    shadowValue = shadow / nbSamples;
}

bool earlyBailing(vec2 coord, float zReceiver, float relativeSearchWidth)
{
    int numBlockers = 0;
    float sumFromLightZ = 0.0;
    
    for (int i=0; i < nbEarlySamples; ++i) {
        float zBlocker = texture(lightMap, earlyBailSamples[i] * relativeSearchWidth + coord).x;
        sumFromLightZ += zBlocker;
        numBlockers += int(mix(0, 1, step(zReceiver, zBlocker)));
    }
    
    // early bailing not applicable for complex surfaces
    if (sumFromLightZ > 1.2)
        return false;
    
    if (numBlockers == 0) {
        shadowValue = 0.0;
        return true;
    } else if (numBlockers == nbEarlySamples) {
        shadowValue = 1.0;
        return true;
    }
    return false;
}
