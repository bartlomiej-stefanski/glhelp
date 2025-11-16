#version 400

in vec3 vPos;
out vec4 color;

void main(void) {
  color = vec4(vPos * 0.8 + vec3(0.2, 0.2, 0.2), 1.0);
}
