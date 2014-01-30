#version 330 core

in vec2 v_uv;
uniform sampler2D element_texture;

layout(location = 0)out vec4 fragColor;

void main()
{
    if(length(v_uv) > 1.0)
        discard;

    fragColor = texture(element_texture,(v_uv*0.5+0.5));
    
    if (length(v_uv) > 0.9)
        fragColor = vec4(1.0,1.0,0.0,1.0)*(1.31-length(v_uv));    
}
