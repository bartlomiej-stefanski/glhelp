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

bool isEqual(float a, float b, float acc) {
  return (a - acc <= b) && (a + acc >= b);
}

vec3 sky_color_from_vector(vec3 view_direction)
{
  vec3 sun_direction = normalize(-dlDirection[0].xyz);
  float sun_adjacency = dot(view_direction, sun_direction);
  float up_direction = max(dot(view_direction, vec3(0.0, 1.0, 0.0)), 0.0);

  if (sun_adjacency >= 0.9999) {
    return vec3(1.0);
  }

  vec3 sky_color = vec3(0.7, 0.78, 1.0);
  vec3 horizon_color = vec3(0.75, 0.74, 0.9);

  return mix(horizon_color, sky_color, up_direction + 0.3);
}

void main(void) {
  vec3 view_direction = normalize(vPos - cameraPosition.xyz);
  vec3 sky_color = sky_color_from_vector(view_direction);

  color = vec4(sky_color, 1.0);
}
