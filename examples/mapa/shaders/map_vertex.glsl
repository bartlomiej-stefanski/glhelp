#version 330
#extension GL_ARB_explicit_uniform_location : require
#extension GL_ARB_shading_language_420pack : require

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 instance_offset;
layout(location = 2) in float texture_layer;

layout (std140) uniform uCommon
{
  mat4 cameraTransform;
  vec4 cameraPosition;
  float time;
};

uniform mat4 uModelTransform;
uniform float map_layer;

out vec3 texturePos;
out vec2 lat_lon;
out float alpha;

const float PI = 3.14159265359;

void main(void)
{
  if (texture_layer < 0) {
    alpha = 0.0;
  }
  else {
    alpha = 1.0;
  }

  texturePos = vec3(pos.x, 1 - pos.y, texture_layer);

  vec3 position = vec3(pos.x * map_layer, pos.y * map_layer, pos.z * map_layer) + instance_offset;
  float deg_y = position.y;
  float mercator_y = degrees(log(tan((PI / 4.0) + radians(deg_y / 2.0))));
  position.y = mercator_y;

  lat_lon = instance_offset.xy + pos.xy * map_layer;
  gl_Position = cameraTransform * uModelTransform * vec4(position, 1.0);
}
