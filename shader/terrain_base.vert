#version 330 core

layout(location = 0)in vec2 _vertex;

out vec2 v_vertex;
out vec3 v_worldPos;
out vec3 v_viewPos;
out vec4 v_projPos;
out vec3 v_normal;

out uint v_isVisible;   // 0==false

uniform mat4 modelTransform;
uniform mat4 modelView;
uniform mat4 modelViewProjection;
uniform samplerBuffer heightField;

uniform uvec2 tileRowsColumns;

void main()
{
    int texIndex = int(_vertex.t) + int(_vertex.s) * int(tileRowsColumns.t); // texelFetch expects an integer position
    float height = texelFetch(heightField, texIndex).x;

    v_vertex = _vertex;
    
    vec4 vertex = vec4(_vertex.s, height, _vertex.t, 1.0);
    v_projPos = modelViewProjection * vertex;
    vec3 normProjPos = v_projPos.xyz / v_projPos.w;
    
    v_isVisible = uint(
        normProjPos.x >= -1 && normProjPos.x <= 1
        && normProjPos.y >= -1 && normProjPos.y <= 1
        && normProjPos.z >= 0 && normProjPos.z <= 1);
    
    v_worldPos = (modelTransform * vertex).xyz;
    
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
}
