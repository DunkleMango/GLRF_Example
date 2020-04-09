#version 460 core
uniform vec3 point_light_position;
uniform float point_light_power;
in vec4 interpolated_color;
out vec4 frag_color;

void main() {
    frag_color = interpolated_color;
}
