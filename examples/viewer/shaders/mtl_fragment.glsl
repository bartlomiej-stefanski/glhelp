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

in vec3 cameraPos;

in vec3 normal;
in vec2 textPos;
in vec3 fragPos;

uniform vec3 uAmbient;
uniform vec3 uDiffuse;
uniform vec3 uSpecular;
uniform float uShininnes;
uniform float uTranslucency;

uniform sampler2D mapAmbient;
uniform sampler2D mapDiffuse;
uniform sampler2D mapSpecular;
uniform sampler2D mapBump;

vec3 AMBIENT;
vec3 DIFFUSE;
vec3 SPECULAR;

out vec4 color;

vec3 calculateDirLight(vec3 Normal, vec3 direction, vec3 viewDir, float diffuse, float specular, vec4 diffuseColor)
{
  vec3 light = vec3(0.0);

  // Diffusef
  float NdotL = max(dot(Normal, -direction), 0.0);
  light += DIFFUSE * diffuseColor.xyz * NdotL * diffuse;

  // Specular
  vec3 reflectDir = reflect(direction, Normal);

  float spec = pow(max(dot(viewDir, reflectDir), 0.0), uShininnes + 1.0);
  light += SPECULAR * spec * specular;

  return light;
}

vec3 calculateSpotLight(
  vec3 Normal,
  vec3 direction,
  vec3 position,
  vec3 fragPos,
  vec3 viewDir,
  vec4 misceleanous,
  float diffuse,
  float specular,
  vec4 diffuseColor)
{
  vec3 light = vec3(0.0);

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
  light += DIFFUSE * diffuseColor.xyz * NdotL * attenuation * intensity * diffuse;

  // Specular
  vec3 reflectDir = reflect(rayDirection, Normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), uShininnes + 1.0);
  light += SPECULAR * spec * attenuation * intensity * specular;

  return light;
}

void main(void)
{
  vec3 Normal = normalize(normal);
  vec3 viewDir = normalize(cameraPos - fragPos);

  vec4 diffuse_texture = texture(mapDiffuse, textPos);

  AMBIENT = uAmbient * texture(mapAmbient, textPos).rgb;
  DIFFUSE = uDiffuse * diffuse_texture.rgb;
  SPECULAR = uSpecular * texture(mapSpecular, textPos).rgb;

  float diffuse_intensity = 1.0;
  float ambient_intensity = 0.001;
  float specular = 1.0;

  vec3 mat_color = vec3(0.0);

  // Calculate directional light effec
  for (int i = 0; i < dlCount; i++) {
    mat_color += calculateDirLight(Normal, dlDirection[i].xyz, viewDir, diffuse_intensity, specular, dlColor[i]);
  }

  // Calculate spot light effects
  for (int i = 0; i < slCount; i++) {
    mat_color += calculateSpotLight(Normal, slDirection[i].xyz, slPosition[i].xyz, fragPos, viewDir, misceleanous[i], diffuse_intensity, specular, slColor[i]);
  }

  mat_color += AMBIENT * ambient_intensity;
  mat_color = pow(mat_color, vec3(1.0 / 2.2)); // SRGB to linear
  color = vec4(mat_color, diffuse_texture[3] * uTranslucency);
}
