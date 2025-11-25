#version 400

in float vtime;
in vec3 vPos;
out vec4 color;

void main(void) {
  vec3 scaled_pos = vPos * 1000;
  vec3 fract_pos = vec3(abs(fract(scaled_pos.x + vtime) - 0.5), abs(fract(scaled_pos.y + vtime) - 0.5), abs(fract(scaled_pos.z + vtime) - 0.5));

  float dist = pow(fract_pos.x, 2) + pow(fract_pos.y, 2) + pow(fract_pos.z, 2);
  if (dist < 0.1) {
    color = vec4(0.0, 0.0, 0.0, 1.0);
  }
  else {
    color = vec4(vPos * 0.8 + vec3(0.2, 0.2, 0.2), 1.0);
  }
}
