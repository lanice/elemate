#version 330 core

uniform sampler2D waterNormals;
uniform sampler2D lavaNormals;
uniform sampler2D sceneColor;
uniform float timef;

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
    vec2 resolution = vec2(0.6,2);
    float x = v_uv.x;
    float y = v_uv.y;
    float mov0 = x+y+cos(sin(timef)*2.)*100.+sin(x/100.)*1000.;
    float mov1 = y / resolution.y / 0.2 + timef;
    float mov2 = x / resolution.x / 0.2;
    float c1 = abs(sin(mov1+timef)/2.+mov2/2.-mov1-mov2+timef);
    float c2 = abs(sin(c1+sin(mov0/1000.+timef)+sin(y/40.+timef)+sin((x+y)/100.)*3.));
    float c3 = abs(sin(c2+cos(mov1+mov2+c2)+cos(mov2)+sin(x/1000.)));
    return vec4(1.0,min(c2,1),0.9*min(c3,1),0);
}