#version 400

uniform float win_time;

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
  vec4 blue = vec4(0.3, 0.3, 1.0, 0.0);
  vec4 black = vec4(0.0, 0.0, 0.0, 0.0);

  float yaw_angle = atan(vPos.z, vPos.x);
  float normalized_yaw = 1 - fract((yaw_angle + 3.14159265) / (2.0 * 3.14159265) + time / 20);
  float pitch_angle = atan(vPos.y, length(vPos.xz));
  float normalized_pitch = 1 - fract((pitch_angle + (3.14159265 / 2.0)) / 3.14159265);
  float sinus_pitch = 2 * (normalized_pitch - 0.5);

  float acc_range = 0.006;
  float divider = 20;
  float red_h = sin(yaw_angle * 5) / divider;
  float green_h = sin(yaw_angle * 5 + (3.14159265 / 3.0) * 2.0) / divider;
  float yellow_h = sin(yaw_angle * 5 + (3.14159265 / 3.0) * 4.0) / divider;
  float blue_h = 5 * sin(yaw_angle * 5) / divider;
  float intensity = pow(normalized_yaw + 0.2, 5);

    if (isEqual(sinus_pitch, blue_h, acc_range)) {
    color = blue * intensity;
  }
  else if (isEqual(sinus_pitch, red_h, acc_range)) {
    color = red * intensity;
  }
  else if (isEqual(sinus_pitch, green_h, acc_range)) {
    color = green * intensity;
  }
  else if (isEqual(sinus_pitch, yellow_h, acc_range)) {
    color = yellow * intensity;
  }
  else if (normalized_pitch < (time - win_time) / 5.0 && win_time > 0.0) {
    color = green;
  }
  else {
    color = black;
  }
}
