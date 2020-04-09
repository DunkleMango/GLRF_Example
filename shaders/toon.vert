#version 460 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out vec3 posWorld, normWorld;

void main() {
  gl_Position = projection * view * model * vec4(position, 1.0);
  posWorld = vec3(model * vec4(position, 1.0));
  normWorld = vec3(transpose(inverse(model)) * vec4(normal, 1.0));
}
