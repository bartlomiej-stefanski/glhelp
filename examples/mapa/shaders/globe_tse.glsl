#version 410 core
#extension GL_ARB_explicit_uniform_location : require
#extension GL_ARB_shading_language_420pack : require

layout (quads, fractional_odd_spacing, ccw) in;

in TCS_OUT {
  vec2 baseLatLon;
  vec2 scaling;
  float layer;
} tes_in[];

layout (std140) uniform uCommon
{
  mat4 cameraTransform;
  vec4 cameraPosition;
  float time;
};

uniform mat4 uModelTransform;
uniform float tile_scale;
uniform float heigthScale;
uniform vec2 camera_lat_lon;

uniform sampler2DArray map;

out vec2 lat_lon;
out vec3 texturePos;

const float PlanetRadius = 6371 * 1000;
const float PI = 3.14159265359;


vec3 LatLonToPoint(float latDeg, float lonDeg, float radius)
{
  float latRad = radians(latDeg);
  float lonRad = radians(lonDeg);

  float y = radius * sin(latRad);
  float rCosLat = radius * cos(latRad);
  float x = rCosLat * sin(lonRad);
  float z = rCosLat * cos(lonRad);

  return vec3(x, y, z);
}

void main()
{
  // The same for all vertices.
  vec2 baseLatLon = tes_in[0].baseLatLon;
  float layer = tes_in[0].layer;
  vec2 scaling = tes_in[0].scaling;

  float u = gl_TessCoord.x * scaling.x;
  float v = gl_TessCoord.y * scaling.y;
  if (tile_scale < 1) {
    u *= tile_scale;
    u += fract(baseLatLon.x);
    v *= tile_scale;
    v += fract(baseLatLon.y);
  }

  float currentLat = baseLatLon.y + (gl_TessCoord.y * scaling.y * tile_scale);
  float currentLon = baseLatLon.x + (gl_TessCoord.x * scaling.x * tile_scale);

  lat_lon = vec2(currentLon, currentLat);

  float dist = distance(vec2(currentLon, currentLat), camera_lat_lon) / tile_scale;
  float heigth_multiplyer = clamp(pow(dist * 10, 2.0), 0.0, 1.0);
  if (tile_scale < 1) {
    heigth_multiplyer = 1.0;
  }

  float rawHeight = 0.0;
  texturePos = vec3(u, 1 - v, layer);
  if (0 < layer && layer < 123456) {
    rawHeight = texture(map, texturePos).r;
  }

  float realHeight = rawHeight * 65535.0 - 500.0;

  vec3 spherePos = LatLonToPoint(currentLat, currentLon, PlanetRadius);
  vec3 sphereNormal = normalize(spherePos);
  vec3 finalPos = spherePos + (sphereNormal * realHeight * heigthScale * heigth_multiplyer);

  gl_Position = cameraTransform * uModelTransform * vec4(finalPos, 1.0);
}
