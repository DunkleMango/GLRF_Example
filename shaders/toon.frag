#version 460 core
uniform vec3 point_light_position;
in vec3 posWorld, normWorld;
out vec4 color;

vec4 toonShade (int c) {
  vec3 col = vec3(1.0, 0.0, 0.0);
  if (c == 0) {
    col = vec3(0.9);
  }
  if (c == 1) {
    col = vec3(0.6);
  }
  if (c == 2) {
    col = vec3(0.3);
  }
  return vec4(col, 1.0);
}

void main() {
  float pi = radians(180);

  vec3 L = normalize(point_light_position - posWorld);
  float LN = acos(dot(L, normalize(normWorld)));

  if (LN >= 0.0 && LN < pi / 6.0) {
    color = toonShade(0);
  } else if (LN >= pi / 6 && LN < pi / 3.0) {
    color = toonShade(1);
  } else {
    color = toonShade(2);
  }

}
