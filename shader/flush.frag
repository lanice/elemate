#version 330 core

in vec2 v_uv;

uniform sampler2D sceneColor;
uniform sampler2D sceneDepth;
uniform sampler2D waterNormals;
uniform sampler2D waterDepth;
uniform sampler2D shadowMap;
uniform sampler2D lightMap;

layout(location = 0)out vec4 fragColor;

float linearize(float depth);
vec4 waterColor();

void main()
{
    fragColor = vec4(texture(shadowMap, v_uv).xxx, 1.0);
    // fragColor = vec4(texture(lightMap, v_uv).xxx, 1.0);
    // fragColor = vec4(texture(waterDepth, v_uv).xxx, 1.0);
    // return;
    // fragColor = texture(sceneColor, v_uv);
    // return;
    float sceneZ = linearize(texture(sceneDepth, v_uv).r);
    float waterZ = texture(waterDepth, v_uv).r;
	fragColor = 
     (texture(shadowMap, v_uv).x * 0.7 + 0.3) * 
    mix(
        // vec4(texture(waterNormals, v_uv).rgb,1.0),
        // vec4(vec3(waterZ),1.0),
        waterColor(),
		texture(sceneColor, v_uv),
		step(sceneZ,waterZ)
	);
}

vec4 waterColor(){
        vec3 resVector = refract(vec3(0,0,-1), texture(waterNormals, v_uv).xyz, 0.8);
        vec4 waterCol = (6*texture(
            sceneColor, 
            v_uv + resVector.xy/10/resVector.z
        )+vec4(0.1,0.8,1,1))/7;

		return mix(
            mix(
                vec4(0,0,0.4,1),
                waterCol,
                0.8+
                0.2*abs(dot(
                    texture(waterNormals, v_uv).xyz,
                    normalize(vec3(0,1,0.1))
                ))
            ),
            vec4(0.9,0.9,0.9,0.8),
            smoothstep(
                0.96,
                0.98,
                dot(
                    texture(waterNormals, v_uv).rgb,
                    normalize(vec3(0,1,0.1))
                )
            )
        );
}