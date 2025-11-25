#version 330
#extension GL_ARB_explicit_uniform_location : require
#extension GL_ARB_shading_language_420pack : require

layout(location = 0) in vec3 pos;
layout(location = 1) in mat4 instanceTransform;

layout (std140) uniform uCommon
{
  mat4 cameraTransform;
  float time;
};

out vec3 vPos;
out float vtime;

uniform mat4 uModelTransform;

void main(void) {
  float angle = time;
  float c = cos(angle);
  float s = sin(angle);

  mat4 rotZ = mat4(
    c, -s, 0.0, 0.0,
    s,  c, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0
  );

  vec4 object_transform = instanceTransform * rotZ * vec4(pos, 1.0);

  vtime = time / 2;
  vPos = object_transform.xyz;
  gl_Position = cameraTransform * uModelTransform * object_transform;
}
