#version 330

out vec3 worldPos;
out vec3 normal;
out vec3 viewPos;

uniform mat4 osg_ModelViewMatrix;
uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat4 modelRotation;

void main()
{
    vec4 viewPos4 = osg_ModelViewMatrix * gl_Vertex;
    viewPos = viewPos4.xyz / viewPos4.w;
    normal = normalize((modelRotation * vec4(gl_Normal, 1.0)).xyz);
    gl_Position = osg_ModelViewProjectionMatrix * gl_Vertex;
}
