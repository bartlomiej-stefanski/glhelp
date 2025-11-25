#version 400

in vec3 normal;
out vec4 color;

void main(void) {
  vec3 light = normalize(vec3(1, 1, 1));
  color = vec4(vec3(1, 1, 1) * (clamp(dot(normal, light), 0, 1) + 0.2), 1);
}
