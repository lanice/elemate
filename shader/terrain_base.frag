#version 330 core

in vec3 g_worldPos;
in vec3 g_normal;
in vec3 g_viewPos;
in vec2 g_rowColumn;
in vec2 g_quadRelativePos;
in float g_temperature;

uniform float zfar;

uniform vec3 lightdir2;
uniform mat4 light2;
uniform vec3 skyColor;

uniform mat4 element_bedrock;
uniform mat4 element_sand;
uniform mat4 element_grassland;

uniform isamplerBuffer terrainTypeID;
uniform sampler2D bedrockSampler;
uniform sampler2D sandSampler;
uniform sampler2D grasslandSampler;

uniform int tileSamplesPerAxis;

uniform bool drawHeatMap;

vec4 phongLighting(vec3 n, vec3 v_pos, vec3 lightdir2, mat4 light2, mat4 material);


layout(location = 0)out vec4 fragColor;

vec3 interpolate(vec2 coeff, vec3 values[4]);

vec3 texColorForElement(int elementId, vec2 texCoord, vec2 texDx, vec2 texDy);
vec3 lightingForElement(int elementId);

void main()
{    
    if (drawHeatMap)
    {
        // float normTemp = (g_temperature + 20.0) / 120.0; // helpful for water
        float normTemp = (g_temperature + 273.0) / 1300.0;  // and for lava
        fragColor = vec4(normTemp, 0, 1.0 - normTemp, 1);
        return;
    }
    
    // get all relevant element ids
    int ids[4] = int[4](// upper left, upper right, lower left, lower right
        texelFetch(terrainTypeID, int(g_rowColumn.t  ) + tileSamplesPerAxis * int(g_rowColumn.s  )).s,
        texelFetch(terrainTypeID, int(g_rowColumn.t  ) + tileSamplesPerAxis * int(g_rowColumn.s+1)).s,
        texelFetch(terrainTypeID, int(g_rowColumn.t+1) + tileSamplesPerAxis * int(g_rowColumn.s  )).s,
        texelFetch(terrainTypeID, int(g_rowColumn.t+1) + tileSamplesPerAxis * int(g_rowColumn.s+1)).s);

    
    vec3 textureColor;
    vec3 lightColor;
    
    // see http://www.opengl.org/wiki/Sampler_%28GLSL%29#Non-uniform_flow_control
    vec2 texCoord = g_worldPos.xz * 0.3;
    vec2 texDx = dFdx(texCoord);
    vec2 texDy = dFdy(texCoord);
    
    if (ids[0] == ids[1] && ids[1] == ids[2] && ids[2] == ids[3]) {
        textureColor = texColorForElement(ids[0], texCoord, texDx, texDy);
        lightColor = lightingForElement(ids[0]);
    }
    else {        
        vec2 texCoeff = mod(g_quadRelativePos + 0.5, 1.0);
        
        vec3 elementTexs[3] = vec3[3](
            textureGrad(bedrockSampler, texCoord, texDx, texDy).rgb,
            textureGrad(sandSampler, texCoord, texDx, texDy).rgb,
            textureGrad(grasslandSampler, texCoord, texDx, texDy).rgb);
            
        vec3 elementLights[3] = vec3[3](
            phongLighting(g_normal, g_viewPos, lightdir2, light2, element_bedrock).rgb,
            phongLighting(g_normal, g_viewPos, lightdir2, light2, element_sand).rgb,
            phongLighting(g_normal, g_viewPos, lightdir2, light2, element_grassland).rgb);
    
        vec3 textureColors[4];
        vec3 lightColors[4];
        for (int i = 0; i < 4; ++i) {
            textureColors[i] = elementTexs[ids[i]];
            lightColors[i] = elementLights[ids[i]];
        }
        textureColor = interpolate(texCoeff, textureColors);
        lightColor = interpolate(texCoeff, lightColors);
    }
    
    vec3 fragColorRgb = mix(lightColor, textureColor, 0.7);
    fragColor = vec4(mix(fragColorRgb, skyColor, // blend at the horizon 
                        // max((gl_FragCoord.z / gl_FragCoord.w - (zfar * 0.9)) / (zfar * 0.1), 0.0)),
                        max(gl_FragCoord.z / (gl_FragCoord.w * 0.1*zfar) - 9, 0.0)),
                    1.0);
                    
    // lava rendering
    // fragColor = mix(fragColor, vec4(1, 0, 0, 1), step(700, g_temperature));
}

vec3 interpolate(vec2 coeff, vec3 values[4])
{
    vec3 v[2] = vec3[2](
        mix(values[0], values[1], coeff.s),
        mix(values[2], values[3], coeff.s));

    return mix(v[0], v[1], coeff.t);
}

vec3 texColorForElement(int elementId, vec2 texCoord, vec2 texDx, vec2 texDy)
{
    if (elementId == 0)
        return textureGrad(bedrockSampler, texCoord, texDx, texDy).rgb;
    if (elementId == 1)
        return textureGrad(sandSampler, texCoord, texDx, texDy).rgb;
    if (elementId == 2)
        return textureGrad(grasslandSampler, texCoord, texDx, texDy).rgb;
    return vec3(1, 0, 0);
}

vec3 lightingForElement(int elementId)
{
    if (elementId == 0)
        return phongLighting(g_normal, g_viewPos, lightdir2, light2, element_bedrock).rgb;
    if (elementId == 1)
        return phongLighting(g_normal, g_viewPos, lightdir2, light2, element_sand).rgb;
    if (elementId == 2)
        return phongLighting(g_normal, g_viewPos, lightdir2, light2, element_grassland).rgb;    
    return vec3(1, 0, 0);
}
