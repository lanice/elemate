#version 330 core

uniform sampler2D waterNormals;
uniform sampler2D waterDepth;
uniform sampler2D lavaNormals;
uniform sampler2D sceneColor;
uniform float timef;
uniform mat4 view;
uniform vec3 camDirection;

float noise(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec4 waterColor(vec2 v_uv){
    vec3 light = normalize(vec3(0.0,1.0,-0.5));
    vec3 normal = (vec4(texture(waterNormals, v_uv).xyz,1.0)*view).xyz;
    vec3 resVector = refract(vec3(0.0,0.0,-1.0),normal , 0.8);
    vec3 waterCol = (5*texture(
                            sceneColor, 
                            v_uv + resVector.xy/10/resVector.z
                    ).rgb
        +vec3(0.1,0.8,1.0))/7.0;

    return vec4(mix(
        mix(
            vec3(0.0,0.0,0.4),
            waterCol,
            0.8+
            0.2*abs(dot(
                normal,
                light
            ))
        ),
        vec3(0.9,0.9,0.9),
        smoothstep(
            0.9,
            1.0,
            dot(
                reflect(light, normal),
                camDirection
            )
        )
    ), 0.5);
}

vec4 lavaColor(vec2 v_uv){
    vec2 resolution = vec2(0.6,2);
    float x = v_uv.x*5;
    float y = v_uv.y*5;
    float mov0 = x+y+cos(sin(timef)*2.)*100.+sin(x/100.)*1000.;
    float mov1 = y / resolution.y / 0.2 + timef;
    float mov2 = x / resolution.x / 0.2;
    float c1 = abs(sin(mov1+timef)/2.+mov2/2.-mov1-mov2+timef);
    float c2 = abs(sin(c1+sin(mov0/1000.+timef)+sin(y/40.+timef)+sin((x+y)/100.)*3.));
    float c3 = abs(sin(c2+cos(mov1+mov2+c2)+cos(mov2)+sin(x/1000.)));
    return vec4(1.0,min(c2,1),0.9*min(c3,1),0);
}