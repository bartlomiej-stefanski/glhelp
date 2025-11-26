#version 400

layout (std140) uniform uDirectionalLights
{
  vec4 Direction[4];
  vec4 Color[4];
  int Count;
};

in vec3 normal;
out vec4 color;

void main(void) {
  vec4 white = vec4(1.0, 1.0, 1.0, 1.0);

  color = white * 0.1; // Ambient light

  for (int i = 0; i < Count; i++) {
    float NdotL = max(dot(normal, -Direction[i].xyz), 0.0);
    color += white * Color[i] * NdotL;
  }
}
