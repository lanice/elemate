#version 410

uniform sampler2D characterAtlas;
uniform vec3 color;

layout (location = 0) out vec4 fragColor;

in vec2 v_textureCoord;

float aastep (float threshold , float value) {
  float afwidth = 0.7 * length(vec2(dFdx(value), dFdy(value)));
  // GLSL 's fwidth(value) is abs(dFdx(value)) + abs(dFdy(value))
  return smoothstep (threshold-afwidth, threshold+afwidth, value);
}

void main() {
    float value = texture(characterAtlas, v_textureCoord).r;
    float alpha = aastep(0.5, value);

    fragColor = vec4(color, alpha);
}
