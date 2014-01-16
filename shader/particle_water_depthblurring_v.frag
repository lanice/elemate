#version 330 core

in vec2 v_uv;

uniform sampler2D source;
uniform ivec2 viewport;

const float coeff[11] = float[11](
    float(184756)/1048576,
    float(167960)/1048576,
    float(125970)/1048576,
    float(77520)/1048576,
    float(38760)/1048576,
    float(15504)/1048576,
    float(4845)/1048576,
    float(1140)/1048576,
    float(190)/1048576,
    float(20)/1048576,
    float(1)/1048576
    );

layout(location = 0)out float depthValue;

void main(){
    ivec2 uv = ivec2(v_uv*vec2(viewport));

    float originDepth = texelFetch(source, uv, 0).r;
    int coeffSize = 10;
    float sum = originDepth*coeff[0];
    for(int i=1; i<=coeffSize; ++i){
            sum += texelFetch(source, uv + ivec2(0, i), 0).r*coeff[i];
            sum += texelFetch(source, uv - ivec2(0, i), 0).r*coeff[i];
    }
    depthValue = mix(sum,1.0,step(1.0,originDepth));
}