#version 460 core
#define MAX_POINT_LIGHTS 10
uniform uint pointLight_count;
uniform vec3 pointLight_position[MAX_POINT_LIGHTS];
uniform vec3 pointLight_color[MAX_POINT_LIGHTS];
uniform float pointLight_power[MAX_POINT_LIGHTS];
uniform vec3 camera_position;

in vec3 interpolated_normal;
in vec3 interpolated_position;

out vec4 frag_color;

void main() {
  vec3 N = normalize(interpolated_normal);
  vec3 V = normalize(camera_position - interpolated_position);
  vec3 R = reflect(-V, N);

  vec3 result_color = vec3(0.0);
  for (int i = 0; i < pointLight_count; i++) {
    vec3 L_denormalized = pointLight_position[i] - interpolated_position;
    float L_dist = length(L_denormalized);
    float falloff = 1.0 / (L_dist * L_dist);

    vec3 L = normalize(L_denormalized);
    float diffuse = max(0.0, dot(L, N));
    float specular = pow(max(dot(L, R), 0.0), 50.0);
    result_color += pointLight_color[i] * (diffuse + specular) * falloff * pointLight_power[i];
  }
  frag_color = vec4(result_color, 1.0);
}
