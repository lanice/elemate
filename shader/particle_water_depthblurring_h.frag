#version 330 core

in vec2 v_uv;

uniform sampler2D source;
uniform ivec2 viewport;
uniform float binomCoeff[465];
uniform int binomOffset[30];

const int coeffSize = 20;

layout(location = 0)out float depthValue;

float calcDepth(float originDepth, ivec2 uv);

void main(){
    ivec2 uv = ivec2(v_uv*vec2(viewport));

    float originDepth = texelFetch(source, uv, 0).r;

    depthValue = mix(
        calcDepth(originDepth,uv),
        1.0,
        step(1.0, originDepth)
    );
}


float calcDepth(float originDepth, ivec2 uv){
    float sum = originDepth * binomCoeff[binomOffset[coeffSize]];
    for(int i=1; i<=coeffSize; ++i){
            sum += (texelFetch(source, uv + ivec2(i, 0), 0).r +
                    texelFetch(source, uv - ivec2(i, 0), 0).r) * binomCoeff[binomOffset[coeffSize] + i];
    }
    return sum;
}