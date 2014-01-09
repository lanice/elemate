#version 330 core

layout(location = 0)in vec2 _vertex;

out vec3 v_viewPos;
out vec3 v_normal;
out float v_heightDiff;

uniform mat4 modelView;
uniform mat4 modelViewProjection;

uniform samplerBuffer heightField;
uniform samplerBuffer baseHeightField;

uniform uvec2 tileRowsColumns;

void main()
{
    int texIndex = int(_vertex.t) + int(_vertex.s) * int(tileRowsColumns.t);
    float height = texelFetch(heightField, texIndex).x;
    float baseHeight = texelFetch(baseHeightField, texIndex).x;
    v_heightDiff = height - baseHeight;
    
    vec4 vertex = vec4(_vertex.x, height, _vertex.y, 1.0);
    
    vec4 viewPos4 = modelView * vertex;
    v_viewPos = viewPos4.xyz / viewPos4.w;
    
    // normal calculation, see http://stackoverflow.com/a/5284527
    float height_left = texelFetch(heightField, texIndex - int(tileRowsColumns.s)).x;
    float height_right = texelFetch(heightField, texIndex + int(tileRowsColumns.s)).x;
    float height_front = texelFetch(heightField, texIndex - 1).x;
    float height_back = texelFetch(heightField, texIndex + 1).x;
    vec3 va =  normalize(vec3(2.0, height_right - height_left, 0.0));
    vec3 vb =  normalize(vec3(0.0, height_back - height_front, 2.0));
    v_normal = - cross(va, vb);
    
    gl_Position = modelViewProjection * vertex;
}
