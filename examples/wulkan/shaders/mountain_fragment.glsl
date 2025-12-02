#version 400

layout (std140) uniform uDirectionalLights
{
  vec4 dlDirection[4];
  vec4 dlColor[4];
  int dlCount;
};

layout (std140) uniform uSpotLights
{
  vec4 slPosition[8];
  vec4 slDirection[8];
  vec4 slColor[8];
  vec4 misceleanous[8]; // x = linear coefficient, y = quadratic coefficient, z = inner cutoff, w = outer cutoff
  int slCount;
};


in vec3 normal;
in vec3 cameraPos;
in vec3 fragPos;

// in vec4 instance_color;

out vec4 color;


float real_noise2(vec2 p)
{
    p = fract(p * vec2(123.3443214, 456.21324));
    p += dot(p, p + 45.3221312);
    return fract(p.x * p.y);
}

float noise(vec2 p)
{
    vec2 i = floor(p * 10);
    vec2 f = fract(p * 10);
    f = f * f * (3.0 - 2.0 * f);
    float res = mix(mix(real_noise2(i + vec2(0.0, 0.0)),
                        real_noise2(i + vec2(1.0, 0.0)),
                        f.x),
                    mix(real_noise2(i + vec2(0.0, 1.0)),
                        real_noise2(i + vec2(1.0, 1.0)),
                        f.x),
                    f.y);
    return res;
}

vec4 calculateDirLight(vec3 Normal, vec3 direction, vec3 viewDir, float diffuse, float specular, vec4 diffuseColor, vec4 matColor)
{
  vec4 light = vec4(0.0);

  // Diffusef
  float NdotL = max(dot(Normal, -direction), 0.0);
  light += matColor * diffuseColor * NdotL * diffuse;

  // Specular
  vec3 reflectDir = reflect(direction, Normal);

  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);
  light += vec4(1.0) * spec * specular;

  return light;
}

vec4 calculateSpotLight(
  vec3 Normal,
  vec3 direction,
  vec3 position,
  vec3 fragPos,
  vec3 viewDir,
  vec4 misceleanous,
  float diffuse,
  float specular,
  vec4 diffuseColor,
  vec4 matColor)
{
  vec4 light = vec4(0.0);

  float distance = length(position - fragPos);
  float attenuation = clamp(1.0 / (1.0 + misceleanous.x * distance + misceleanous.y * (distance * distance)), 0.0, 1.0);
  vec3 rayDirection = normalize(fragPos - position);

  /* Calculate intensity of cutoff, where:
    * - theta: angle between light direction and ray direction
    * - slope: rate of change from full intensity to no intensity */
  float inner_cutoff = cos(misceleanous.z);
  float outer_cutoff = cos(misceleanous.w);

  float theta = dot(rayDirection, -direction);
  float slope = inner_cutoff - outer_cutoff + 0.00001; // Avoid division by zero

  float intensity = clamp((theta - outer_cutoff) / slope, 0.0, 1.0);

  // Diffuse
  float NdotL = max(dot(Normal, -rayDirection), 0.0);
  light += matColor * diffuseColor * NdotL * attenuation * intensity * diffuse;

  // Specular
  vec3 reflectDir = reflect(rayDirection, Normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);
  light += vec4(1.0) * spec * attenuation * intensity * specular;

  return light;
}

vec4 add_fog(float camera_distance, vec4 color)
{
  float fog_factor = 1.0 - exp(-camera_distance * 0.01);
  fog_factor = clamp(fog_factor, 0.0, 1.0);
  vec3 fog_color = vec3(0.5, 0.6, 0.7);

  return color = mix(color, vec4(fog_color, 1.0), fog_factor);
}

void main(void)
{
  float camera_distance = max(length(cameraPos - fragPos) / 10, 1.0);
  float detailNoise = noise(fragPos.xz * 0.5);
  float heightMapVal = detailNoise;
  vec3 bumpNormal = normalize(vec3(
      dFdx(heightMapVal) * 2.0 / camera_distance, // Siła chropowatości X
      1.0,                       // Oś Y bez zmian
      dFdy(heightMapVal) * 2.0 / camera_distance  // Siła chropowatości Z
  ));

  vec3 Normal = normalize(normal + (bumpNormal * 0.2));
  vec3 viewDir = normalize(cameraPos - fragPos);

  float up_amount = dot(Normal, vec3(0, 1, 0));

  // Determine terrain type, default one are rocks
  float diffuse = 1.0;
  float ambient = 0.1;
  float specular = 0.02;
  vec4 matColor = vec4(0.3, 0.3, 0.3, 1.0);
  if (fragPos.y + up_amount * 10 > 203) /* snow */ {
    specular = 1.0;
    matColor.xyz = vec3(1.0);
  }
  else if (fragPos.y - up_amount * 50 < 50) /* grass */ {
    matColor.xyz = vec3(0.4, 0.8, 0.3);
    matColor.xyz += vec3(noise1(ceil(fragPos.x)), noise1(ceil(fragPos.y)), noise1(ceil(fragPos.z)));
  }

  color = vec4(vec3(0.0), 1.0);

  // Calculate directional light effec
  for (int i = 0; i < dlCount; i++) {
    color += calculateDirLight(Normal, dlDirection[i].xyz, viewDir, diffuse, specular, dlColor[i], matColor);
  }

  // Calculate spot light effects
  for (int i = 0; i < slCount; i++) {
    color += calculateSpotLight(Normal, slDirection[i].xyz, slPosition[i].xyz, fragPos, viewDir, misceleanous[i], diffuse, specular, slColor[i], matColor);
  }

  color += matColor * ambient;
  color = add_fog(camera_distance, color);
  color.rgb = pow(color.rgb, vec3(1.0 / 2.2)); // SRGB to linear
}
