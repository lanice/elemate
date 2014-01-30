#version 330 core

in vec2 v_uv;

layout(location = 0)out vec4 fragColor;

void main()
{
    if(length(v_uv) > 1.0)
        discard;
    fragColor = vec4(0.4f,0.4f,0.0f,1.0f);
}
