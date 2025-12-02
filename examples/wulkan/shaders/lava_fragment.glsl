#version 400

in vec3 vPos;

out vec4 color;

layout (std140) uniform uCommon
{
  mat4 cameraTransform;
  vec4 cameraPosition;
  float time;
};

layout (std140) uniform uDirectionalLights
{
  vec4 dlDirection[4];
  vec4 dlColor[4];
  int dlCount;
};

void main(void) {
  color = vec4(1,0,0,1);
}
