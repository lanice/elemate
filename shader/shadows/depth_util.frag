#version 330 core

// from https://github.com/hpicgs/cgsee

uniform ivec2 viewport;
uniform float znear;
uniform float zfar;

float linearize(float depth) {
	// d = (2.0 * zfar * znear / (zfar + znear - (zfar - znear) * (2.0 * z- 1.0)));
	// normalized to [0,1]
	// d = (d - znear) / (zfar - znear);

	// simplyfied with wolfram alpha
	return - znear * depth / (zfar * depth - zfar - znear * depth);
}