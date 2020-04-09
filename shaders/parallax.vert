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
uniform vec3 camera_position;

struct VS_OUT {
  vec2 texcoord;
  vec3 P;
  vec3 N;
  mat3 TBN;
  vec3 tangent_point_light_positions[MAX_POINT_LIGHTS];
  vec3 tangent_camera_position;
  vec3 tangent_P;
};
out VS_OUT VS;

void main() {
  vec4 P_World_homogenous = model * vec4(in_position, 1.0);
  gl_Position = projection * view * P_World_homogenous;
  VS.P = P_World_homogenous.xyz / P_World_homogenous.w;
  VS.N = model_normal * in_normal;
  VS.texcoord = in_uv;

  vec3 T = normalize(model_normal * normalize(in_tangent));
  vec3 N = normalize(model_normal * normalize(in_normal));
  vec3 B = normalize(model_normal * cross(in_tangent, in_normal));
  VS.TBN = transpose(mat3(T, B, N));
  for (uint i = 0; i < pointLight_count; i++) {
		VS.tangent_point_light_positions[i] = VS.TBN * pointLight_position[i];
	}
  VS.tangent_camera_position = VS.TBN * camera_position;
  VS.tangent_P = VS.TBN * VS.P;
}
