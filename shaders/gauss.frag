#version 460 core
uniform sampler2D image;

uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
uniform bool horizontal;
uniform bool first_iteration;

in vec2 interpolated_uv;

out vec4 frag_color;

void main() {
  vec2 uv_offset = 1.0 / textureSize(image, 0); // gets size of single texel
  vec3 result = texture(image, interpolated_uv).rgb * weight[0]; // current fragment's contribution
  if(horizontal) {
      for(int i = 1; i < 5; ++i) {
          result += texture(image, interpolated_uv + vec2(uv_offset.x * i, 0.0)).rgb * weight[i];
          result += texture(image, interpolated_uv - vec2(uv_offset.x * i, 0.0)).rgb * weight[i];
      }
  }
  else {
      for(int i = 1; i < 5; ++i) {
          result += texture(image, interpolated_uv + vec2(0.0, uv_offset.y * i)).rgb * weight[i];
          result += texture(image, interpolated_uv - vec2(0.0, uv_offset.y * i)).rgb * weight[i];
      }
  }
  frag_color = vec4(result, 1.0);
}
