#version 330
#extension GL_ARB_explicit_uniform_location : require
#extension GL_ARB_shading_language_420pack : require

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;

/* {{position.xyz},
 *  {hsl_color, time_offset, ...},
 *  {max_heigth, start_heigth, ...}}*/
layout(location = 2) in mat3 instance_data;

layout (std140) uniform uCommon
{
  mat4 cameraTransform;
  vec4 cameraPosition;
  float time;
};

uniform mat4 uModelTransform;
uniform mat3 uNormalTransform;

out vec3 normal;
out vec3 cameraPos;
out vec3 fragPos;
out vec4 instance_color;

// Tutorial do zamiany hue do RGB: https://www.baeldung.com/cs/convert-color-hsl-rgb
vec3 hue2rgb(float h, float c, float x) {
  if(h < 1.0) return vec3(c, x, 0.0);
  else if(h < 2.0) return vec3(x, c, 0.0);
  else if(h < 3.0) return vec3(0.0, c, x);
  else if(h < 4.0) return vec3(0.0, x, c);
  else if(h < 5.0) return vec3(x, 0.0, x);
  else return vec3(c, 0.0, x);
}

vec3 hsl2rgb(vec3 hsl) {
  float h = hsl.x;
  float s = hsl.y;
  float l = hsl.z;

  float r, g, b;

  float c =  (1.0 - abs(2.0 * l - 1.0)) * s;
  float hp = h / 60.0f;
  float x = c * (1.0 - abs(mod(hp, 2.0) - 1.0));

  float m = l - c / 2.0;
  return hue2rgb(hp, c, x) + vec3(m, m, m);
}

void main(void) {
  normal = uNormalTransform * norm;
  cameraPos = cameraPosition.xyz;

  float max_heigth = instance_data[2].x;
  float start_heigth = instance_data[2].y;
  float time_offset = instance_data[1].y;


  instance_color = vec4(hue2rgb(instance_data[1].x, 1.0, 1.0), 0.3);

  float vertical_transition = (max_heigth - start_heigth) / 2;


  vec3 movedInstancePosition = pos + instance_data[0];
  movedInstancePosition.y += 10 * (sin(time + time_offset) + 1 + start_heigth) * vertical_transition;

  vec4 world_position = uModelTransform * vec4(pos + movedInstancePosition, 1.0);
  fragPos = vec3(world_position);
  gl_Position = cameraTransform * world_position;
}
