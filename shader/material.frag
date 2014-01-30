#version 330 core

uniform sampler2D waterNormals;
uniform sampler2D lavaNormals;
uniform sampler2D sceneColor;

vec4 waterColor(vec2 v_uv){
        vec3 resVector = refract(vec3(0,0,-1), texture(waterNormals, v_uv).xyz, 0.8);
        vec3 waterCol = (6*texture(
                                sceneColor, 
                                v_uv + resVector.xy/10/resVector.z
                        ).rgb
            +vec3(0.1,0.8,1))/7;

		return vec4(mix(
            mix(
                vec3(0,0,0.4),
                waterCol,
                0.8+
                0.2*abs(dot(
                    texture(waterNormals, v_uv).xyz,
                    normalize(vec3(0,1,0.1))
                ))
            ),
            vec3(0.9,0.9,0.9),
            smoothstep(
                0.96,
                0.98,
                dot(
                    texture(waterNormals, v_uv).rgb,
                    normalize(vec3(0,1,0.1))
                )
            )
        ), 0.5);
}

vec4 lavaColor(vec2 v_uv){
    return vec4(0.5,0.1,0.1,0);
}