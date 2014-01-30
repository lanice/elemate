#version 330 core

in vec2 v_uv;
uniform sampler2D element_texture;

layout(location = 0)out vec4 fragColor;

void main()
{
    if(length(v_uv) > 1.0)
        discard;

    fragColor = texture(element_texture,(v_uv*0.5+0.5)*0.1);
    
    if (length(v_uv) > 0.95)
        fragColor = vec4(0);    
}
