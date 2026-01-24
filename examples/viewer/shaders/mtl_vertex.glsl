#version 330
#extension GL_ARB_explicit_uniform_location : require
#extension GL_ARB_shading_language_420pack : require

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 text;

layout (std140) uniform uCommon
{
  mat4 cameraTransform;
  vec4 cameraPosition;
  float time;
};

uniform mat4 uModelTransform;
uniform mat3 uNormalTransform;

out vec3 cameraPos;

out vec3 normal;
out vec2 textPos;
out vec3 fragPos;

void main(void) {
  cameraPos = cameraPosition.xyz;

  normal = uNormalTransform * norm;
  textPos = text;

  vec4 world_position = uModelTransform * vec4(pos, 1.0);
  fragPos = vec3(world_position);
  gl_Position = cameraTransform * world_position;
}
