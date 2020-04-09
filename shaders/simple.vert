#version 460 core
layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
uniform mat4 model;
uniform mat3 model_normal;
uniform mat4 view;
uniform mat4 projection;

out vec3 N, P;

void main() {
    gl_Position = projection * view * model * vec4(in_position, 1.0);
    vec4 P_homogenous = view * model * vec4(in_position, 1.0);
    vec3 P = P_homogenous.xyz / P_homogenous.w;
    N = normalize(model_normal * in_normal);
}
