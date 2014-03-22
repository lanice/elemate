#version 330 core

in vec2 v_uv;
uniform sampler2D element_texture;
uniform int highlighted;

layout(location = 0)out vec4 fragColor;

void main()
{
    if(length(v_uv) > 1.0)
        discard;

    fragColor = mix(vec4(vec2(1.31-length(v_uv)),
                                0.3+(highlighted * 0.5),
                                1.0), 
                    texture(element_texture,(v_uv*0.5+0.5)), 
                    step(length(v_uv),0.8));
}
