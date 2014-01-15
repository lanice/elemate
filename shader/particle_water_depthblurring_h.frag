#version 330 core

in vec2 v_uv;

uniform sampler2D source;
uniform ivec2 viewport;

layout(location = 0)out float depthValue;

void main(){
    ivec2 uv = ivec2(v_uv*vec2(viewport));
    
    int coeffSize = 31;
    int used = 0;
    float sum = 0;
    for(int i=-coeffSize; i<=coeffSize; ++i){
        float depth = texelFetch(source, uv + ivec2(i, 0), 0).r;
        if(depth < 100.0){
            sum += depth;
            used += 1;
        }
    }

    depthValue = sum/max(used,1.0);
}