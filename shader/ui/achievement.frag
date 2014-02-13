#version 330 core

in vec2 v_uv;
uniform sampler2D picture;

layout(location = 0)out vec4 fragColor;

void main()
{
	if(v_uv.x < -0.3 && v_uv.x > -0.95 &&
	   v_uv.y < 0.8 && v_uv.y > -0.9)
		fragColor = texture(picture,(v_uv*0.5+0.5));   
	else
		fragColor = vec4(0.3);
}
