#version 330 core

uniform sampler2D particleNormals;
uniform sampler2D particleDepth;
uniform sampler2D lavaNormals;
uniform sampler2D sceneColor;
uniform int timef;
uniform mat4 view;
uniform vec3 camDirection;

vec4 waterColor(vec2 v_uv, float worldHeight)
{
    vec3 light = normalize(vec3(0.0,1.0,-0.5));
    vec3 normal = (vec4(texture(particleNormals, v_uv).xyz,1.0)*view).xyz;
    vec3 resVector = refract(vec3(0.0,0.0,-1.0),normal , 0.8);
    float depth = texture(particleDepth, v_uv).x;
    
    vec4 sceneC = texture(
        sceneColor, 
        v_uv - max(0.5-depth,0.0)*resVector.xy/10/resVector.z);
        
    // we can't see the oceans ground
    float sceneToBackgroundLimit = -10;
        
    // fragColor = vec4(
    // smoothstep(-20, -8, worldHeight - 10.0)
    // , 0.0, 0.0, 1.0);
    // return;
    vec3 waterCol = (
        5 * mix(vec3(0, 0.1, 0.3), sceneC.rgb, smoothstep(-18, -10, worldHeight))
        + vec3(0.1, 0.8, 1.0))
        * 0.14286 /* /7 */;

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
    ), 1.0);
}

vec4 lavaColor(vec2 v_uv)
{
    /* vec2 resolution = vec2(0.6,2);
    float x = v_uv.x*5;
    float y = v_uv.y*5;
    float mov0 = x+y+cos(sin(timef)*2.)*100.+sin(x/100.)*1000.;
    float mov1 = y / resolution.y / 0.2 + timef;
    float mov2 = x / resolution.x / 0.2;
    float c1 = abs(sin(mov1+timef)/2.+mov2/2.-mov1-mov2+timef);
    float c2 = abs(sin(c1+sin(mov0/1000.+timef)+sin(y/40.+timef)+sin((x+y)/100.)*3.));
    float c3 = abs(sin(c2+cos(mov1+mov2+c2)+cos(mov2)+sin(x/1000.)));
    return vec4(1.0,min(c2,1),0.9*min(c3,1),0); */


    vec3 light = vec3(0.0, 0.894427, -0.444721); // = normalize(vec3(0.0,1.0,-0.5));
    vec3 normal = (texture(particleNormals, v_uv) * view).xyz;
    vec3 lavaCol = vec3(0.5,0.01,0.01);

    return vec4(mix(
        mix(
            vec3(0.1,0.0,0.0),
            lavaCol,
            0.2+
            0.8*max(
                    dot(normal,light),
                    0.0
                )
        ),
        vec3(1.0,0.8,0.1),
        smoothstep(
            0.9,
            1.0,
            dot(
                reflect(light, normal),
                camDirection
            )
        )
        ), 0.0);
}

const vec3 baseSandColor = vec3(0.776, 0.741, 0.608);

vec4 sandColor(vec2 v_uv)
{
    float x = gl_FragCoord.x*sin(gl_FragCoord.x);
    float y = gl_FragCoord.y*cos(gl_FragCoord.y);
    float rand_val = 1.0 - clamp(int(x+y)*100 %(int(x-y)%2) * 0.0001,0.0,0.2);
    x = 123+x;
    y = 321+y;
    rand_val = rand_val - clamp(int(x+y)*100 %(int(x-y)%5) * 0.0001,0.0,0.25);
    x = 321+x;
    y = 123+y;
    rand_val = rand_val - clamp(int(x+y)*100 %(int(x-y)%10) * 0.0001,0.0,0.3);

    vec3 light   = normalize(vec3(0.0,1.0,-0.5));
    vec3 normal  = (vec4(texture(particleNormals, v_uv).xyz,1.0)*view).xyz;
    vec3 sandCol = baseSandColor * vec3(rand_val);

    return vec4(
        mix(
            baseSandColor * (1 - rand_val),
            sandCol,
            0.5+
            0.5*max(
                    dot(normal,light),
                    0.0
            )
        ), 
        1.0
    );
}

vec4 steamColor(vec2 v_uv)
{
    vec3 background = texture(sceneColor,v_uv).rgb;
    vec3 light = normalize(vec3(0.0,1.0,-0.5));
    vec3 normal = (vec4(texture(particleNormals, v_uv).xyz,1.0)*view).xyz;
    //return vec4(mix(background,vec3(0.8),0.5),1.0);

    return vec4(
        mix(
            vec3(0.9),
            background,
            0.2+
            0.8*abs(dot(
                normal,
                vec3(0,0,-1.0)
            ))
        ), 1.0);
}