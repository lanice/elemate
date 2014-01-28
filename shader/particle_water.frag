#version 330 core

in vec2 g_relPos;
flat in float g_zDiff;
in vec4 g_viewPos;
flat in vec4 g_viewPosCenter;

uniform float particleSize;
uniform mat4 projection;

float linearize(float depth);
float depthNdcToWindow(float ndcDepth);

void main()
{        
    vec3 N;
    N.xy = g_relPos.xy;
    float r2 = dot(N.xy, N.xy);
    if (r2 > 1.0)
        discard;
    
    N.z = sqrt(1.0 - r2);
    
    vec4 viewFragPos = g_viewPosCenter + vec4(N * particleSize, 0.0);
    vec4 screenFragPos = projection * viewFragPos;
    float ndcDepth = screenFragPos.z / screenFragPos.w;
    
    gl_FragDepth = linearize(depthNdcToWindow(ndcDepth));
}
