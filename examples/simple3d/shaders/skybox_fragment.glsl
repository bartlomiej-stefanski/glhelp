#version 400

in vec3 vPos;
out vec4 color;

layout (std140) uniform uCommon
{
  mat4 cameraTransform;
  float time;
};

bool isEqual(float a, float b, float acc) {
  return (a - acc <= b) && (a + acc >= b);
}

void main(void) {
  vec4 red = vec4(0.8, 0.0, 0.0, 0.0);
  vec4 green = vec4(0.0, 0.8, 0.0, 0.0);
  vec4 yellow = vec4(0.8, 0.8, 0.0, 0.0);
  vec4 black = vec4(0.0, 0.0, 0.0, 0.0);

  float yaw_angle = atan(vPos.z, vPos.x);
  float normalized_angle = 1 - fract((yaw_angle + 3.14159265) / (2.0 * 3.14159265) + time / 20);

  float acc_range = 0.01;
  float divider = 10;
  float red_h = sin(yaw_angle * 5) / divider;
  float green_h = sin(yaw_angle * 5 + (3.14159265 / 3.0) * 2.0) / divider;
  float yellow_h = sin(yaw_angle * 5 + (3.14159265 / 3.0) * 4.0) / divider;
  float intensity = pow(normalized_angle + 0.2, 5);

  if (isEqual(vPos.y, red_h, acc_range)) {
    color = red * intensity;
  }
  else if (isEqual(vPos.y, green_h, acc_range)) {
    color = green * intensity;
  }
  else if (isEqual(vPos.y, yellow_h, acc_range)) {
    color = yellow * intensity;
  }
  else {
    color = black;
  }
}
