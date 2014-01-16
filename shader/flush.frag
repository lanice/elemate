#version 330 core

in vec2 v_uv;

uniform sampler2D sceneColor;
uniform sampler2D sceneDepth;
uniform sampler2D waterNormals;
uniform sampler2D waterDepth;

layout(location = 0)out vec4 fragColor;

void main()
{
    float sceneZ = texture(sceneDepth, v_uv).r;
    float waterZ = texture(waterDepth, v_uv).r;
	fragColor = mix(
		mix(
            mix(
                vec4(vec3(0.0),0.8),
                vec4(0.2, 0.3, 0.8, 0.8),
                dot(
                    texture(waterNormals, v_uv).rgb,
                    normalize(vec3(0,1,0.5))
                )
            ),
            vec4(0.5,0.7,0.9,0.8),
            smoothstep(
                0.95,
                0.98,
                dot(
                    texture(waterNormals, v_uv).rgb,
                    normalize(vec3(0,1,0.5))
                )
            )
        ),
		texture(sceneColor, v_uv),
		step(sceneZ,waterZ)
	);
}
