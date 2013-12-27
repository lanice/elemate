#version 330 core

layout(location = 0)in vec2 _vertex;

out vec2 v_vertex;
out vec3 v_worldPos;
out vec3 v_viewPos;
out vec4 v_projPos;

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
    
    vec4 vertex = vec4(_vertex.x, height, _vertex.y, 1.0);
    vec4 _worldPos = modelTransform * vertex;
    v_worldPos = _worldPos.xyz / _worldPos.w;
    
    vec4 viewPos4 = modelView * vertex;
    v_viewPos = viewPos4.xyz / viewPos4.w;
    
    v_projPos = modelViewProjection * vertex;
}
