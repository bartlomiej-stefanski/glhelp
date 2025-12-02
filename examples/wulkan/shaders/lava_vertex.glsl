#version 330
#extension GL_ARB_explicit_uniform_location : require
#extension GL_ARB_shading_language_420pack : require

layout(location = 0) in vec3 pos;

layout (std140) uniform uCommon
{
  mat4 cameraTransform;
  vec4 cameraPosition;
  float time;
};

out vec3 vPos;

uniform mat4 uModelTransform;

void main(void) {
  vPos = (uModelTransform * vec4(pos, 1.0)).xyz;
  gl_Position = cameraTransform * uModelTransform * vec4(pos, 1.0);
}
