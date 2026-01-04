#version 410 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 instance_offset;
layout (location = 2) in float texture_layer;

out VS_OUT {
  vec2 baseLatLon;
  vec2 scaling;
  float layer;
} vs_out;

uniform float tile_scale;
uniform vec2 start_point;

void main()
{
  vec2 offset = instance_offset.xy * tile_scale + start_point;

  // Some larger tiles with edges (tile_scale) that do not divide 180 or 360 might not 'fit the globe'.
  // For example a tile with an edge of 16 degrees might start at 352 and end at 368 - it will overlap with the first tile!
  // To compensate for this the tiles lat/lon is checked here and 'scaled' appropriatley.
  vec2 max_lat_lon = vec2(180, 90);
  vec2 curr_max_lat_lon = offset + vec2(tile_scale, tile_scale);
  vs_out.scaling = 1.0 - max(vec2(0,0), (curr_max_lat_lon - max_lat_lon) / tile_scale);

  vs_out.baseLatLon = offset;
  vs_out.layer = texture_layer;
}
