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


vec4 calculateDirLight(vec3 Normal, vec3 direction, vec3 viewDir, float diffuse, float specular, vec4 diffuseColor, vec4 matColor)
{
  vec4 light = vec4(0.0);

  // Diffusef
  float NdotL = max(dot(Normal, -direction), 0.0);
  light += matColor * diffuseColor * NdotL * diffuse;

  // Specular
  vec3 reflectDir = reflect(direction, Normal);

  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 256);
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
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 256);
  light += vec4(1.0) * spec * attenuation * intensity * specular;

  return light;
}

// TODO: add fresnel
void main(void) {
  vec3 Normal = normalize(normal);
  vec3 viewDir = normalize(cameraPos - fragPos);

  float diffuse = 1.0;
  float ambient = 0.1;
  float specular = 0.8;

  vec4 matColor = vec4(0.7, 0.7, 0.1, 1.0);

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
  color.rgb = pow(color.rgb, vec3(1.0 / 2.2)); // SRGB to linear
}
