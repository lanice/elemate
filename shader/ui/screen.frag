#version 330 core

in vec2 v_uv;

layout(location = 0)out vec4 fragColor;

void main()
{
    float strength = mix(0.4,0.1,smoothstep(0.4,1.5,length(v_uv)));
    fragColor = vec4(strength,strength,strength,0.9);    
}
