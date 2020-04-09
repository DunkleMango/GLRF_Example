#version 460 core
#define MAX_POINT_LIGHTS 10
layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_uv;
layout (location = 3) in vec3 in_tangent;

uniform mat4 model;
uniform mat3 model_normal;
uniform mat4 view;
uniform mat4 projection;
uniform uint pointLight_count;
uniform vec3 pointLight_position[MAX_POINT_LIGHTS];

out vec3 interpolated_normal;
out vec3 interpolated_position;
out vec2 interpolated_uv;
out mat3 TBN;

void main() {
  mat4 MV = view * model;
  gl_Position = projection * MV * vec4(in_position, 1.0);
  vec4 P_homogenous = MV * vec4(in_position, 1.0);
  interpolated_position = P_homogenous.xyz / P_homogenous.w;
  interpolated_normal = normalize(model_normal * in_normal);
  interpolated_uv = in_uv;

  vec3 T = normalize(vec3(MV * vec4(in_tangent,   0.0)));
  vec3 N = normalize(vec3(MV * vec4(in_normal,    0.0)));
  vec3 B = cross(N, T);
  TBN = mat3(T, B, N);
}
