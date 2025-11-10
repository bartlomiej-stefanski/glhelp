R"END(

#version 330
#extension GL_ARB_explicit_uniform_location : require
#extension GL_ARB_shading_language_420pack : require

layout(location = 0) in vec3 pos;

layout (std140) uniform uCommon
{
  mat4 cameraTransform;
  float time;
};

out vec4 vColor;

uniform mat4 uModelTransform;

void main(void) {
  vec4 red = vec4(1.0, 0.0, 0.0, 1.0);
  vec4 green = vec4(0.0, 1.0, 0.0, 1.0);
  vec4 blue = vec4(0.0, 0.0, 1.0, 1.0);

  vec4 colors[] = {red, green, blue, red, green, blue, red, green};

  vColor = colors[gl_VertexID];
  gl_Position = cameraTransform * uModelTransform * vec4(pos, 1.0);
}

)END"
