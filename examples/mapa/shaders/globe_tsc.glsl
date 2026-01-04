#version 410 core
#extension GL_ARB_explicit_uniform_location : require
#extension GL_ARB_shading_language_420pack : require

layout (vertices = 4) out;

layout (std140) uniform uCommon
{
  mat4 cameraTransform;
  vec4 cameraPosition;
  float time;
};

in VS_OUT {
  vec2 baseLatLon;
  vec2 scaling;
  float layer;
} tcs_in[];

out TCS_OUT {
  vec2 baseLatLon;
  vec2 scaling;
  float layer;
} tcs_out[];

uniform float tile_scale;
uniform vec2 camera_lat_lon;

vec3 LatLonToUnitVector(float latDeg, float lonDeg) {
    float latRad = radians(latDeg);
    float lonRad = radians(lonDeg);

    // Wzór na sferę o promieniu R=1
    float y = sin(latRad);
    float rCosLat = cos(latRad);
    float x = rCosLat * sin(lonRad);
    float z = rCosLat * cos(lonRad);

    return vec3(x, y, z);
}

// Guaranteed to be at least 64.0 by
uniform float MaxTess = 64.0;
const float radius_km = 6371.0;
const float radius = radius_km * 1000.0;

void main()
{
  tcs_out[gl_InvocationID].baseLatLon = tcs_in[gl_InvocationID].baseLatLon;
  tcs_out[gl_InvocationID].scaling    = tcs_in[gl_InvocationID].scaling;
  tcs_out[gl_InvocationID].layer      = tcs_in[gl_InvocationID].layer;

  if (gl_InvocationID == 0) {
    vec2 curr_lat_lon = tcs_in[gl_InvocationID].baseLatLon + vec2(tile_scale * 0.5);

    float dist = distance(curr_lat_lon, camera_lat_lon) / tile_scale;
    float factor = 1.0 - clamp(dist / 10.0, 0.000, 0.9);
    float tess_level = max(1.0, MaxTess * factor);

    gl_TessLevelOuter[0] = tess_level;
    gl_TessLevelOuter[1] = tess_level;
    gl_TessLevelOuter[2] = tess_level;
    gl_TessLevelOuter[3] = tess_level;

    gl_TessLevelInner[0] = tess_level;
    gl_TessLevelInner[1] = tess_level;
  }
}
