#version 330
#extension GL_ARB_explicit_uniform_location : require
#extension GL_ARB_shading_language_420pack : require

layout(location = 0) in vec3 pos;

layout (std140) uniform uCommon
{
  mat4 cameraTransform;
  float time;
};

uniform mat4 uModelTransform;

void main(void) {
  gl_Position = cameraTransform * uModelTransform * vec4(pos, 1.0);
}
