#version 460 core
in vec3 N, P;
out vec4 frag_color;

void main() {
    frag_color = vec4(N, 1.0);
}
