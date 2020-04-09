#version 460 core
layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
uniform mat4 model;
uniform mat3 model_normal;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 point_light_position;
out vec4 interpolated_color;

void main() {
    vec3 light_color = vec3(1.0, 1.0, 1.0);

    gl_Position = projection * view * model * vec4(in_position, 1.0);
    vec4 P_homogenous = view * model * vec4(in_position, 1.0);
    vec3 P = P_homogenous.xyz / P_homogenous.w;
    vec3 N = normalize(model_normal * in_normal);
    vec3 L = point_light_position - P;
    float L_dist = length(L);
    vec3 kd = light_color * max(0.0, dot(normalize(L), normalize(N))) / (L_dist * L_dist);
    interpolated_color = vec4(kd, 1.0);
}
