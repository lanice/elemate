#version 330 core

// from https://github.com/hpicgs/cgsee

uniform float znear;
uniform float zfar;

float linearize(float depth) {
    // d = (2.0 * zfar * znear / (zfar + znear - (zfar - znear) * (2.0 * z- 1.0)));
    // normalized to [0,1]
    // d = (d - znear) / (zfar - znear);

    // simplyfied with wolfram alpha
    return - znear * depth / (zfar * depth - zfar - znear * depth);
}

float depthNdcToWindow(float ndcDepth)
{   // https://www.opengl.org/wiki/Vertex_Post-Processing#Viewport_transform
    return ((gl_DepthRange.diff * ndcDepth) + gl_DepthRange.near + gl_DepthRange.far) / 2.0;
}
