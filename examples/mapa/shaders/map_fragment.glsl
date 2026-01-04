#version 400

in vec3 texturePos;
in vec2 lat_lon;
in float alpha;

out vec4 color;

uniform vec2 draw_from;
uniform vec2 draw_to;
uniform sampler2DArray map;

vec3 hue2rgb(float h, float c, float x)
{
  if(h < 1.0) return vec3(c, x, 0.0);
  else if(h < 2.0) return vec3(x, c, 0.0);
  else if(h < 3.0) return vec3(0.0, c, x);
  else if(h < 4.0) return vec3(0.0, x, c);
  else if(h < 5.0) return vec3(x, 0.0, x);
  else return vec3(c, 0.0, x);
}

vec3 hsl2rgb(vec3 hsl)
{
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


void main(void)
{
  vec3 terrain_color = vec3(0.0);

  bool is_in_region = (draw_from.x <= lat_lon.x && lat_lon.x <= draw_to.x)
                  && (draw_from.y <= lat_lon.y && lat_lon.y <= draw_to.y);

  float real_alpha = alpha;

  if (texturePos.z < 123456 && is_in_region) {
    float ht = (texture(map, texturePos).r * 65535.0) - 500.0;
    if      (ht <= 0  )   terrain_color = vec3(0.,       0.,        1.); //blue
    else if (ht <= 500)   terrain_color = vec3(0.,       ht/500,    0.); //->green
    else if (ht <= 1000)  terrain_color = vec3(ht/500-1, 1.,        0.); //->yellow
    else if (ht <= 2000)  terrain_color = vec3(1.,       2.-ht/1000,0.); //->red
    else if (ht <= 9000)  terrain_color = vec3(1.,       ht/2000-1 ,ht/2000-1);  //->white
  }
  else {
    float stepX = 1.0 - step(0.02, fract(lat_lon.x / 4));
    float stepY = 1.0 - step(0.02, fract(lat_lon.y / 4));
    float step = min(1.0, stepX + stepY);
    terrain_color = vec3(step, step, 1);
    if (!is_in_region) {
      real_alpha = 1.0;
    }
  }

  color = vec4(terrain_color, real_alpha);
  color.rgb = pow(color.rgb, vec3(1.0 / 2.2)); // SRGB to linear
}
