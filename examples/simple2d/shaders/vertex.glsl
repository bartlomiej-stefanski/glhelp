R"END(

#version 330
#extension GL_ARB_explicit_uniform_location : require
#extension GL_ARB_shading_language_420pack : require

layout(location = 0) in vec2 pos;

uniform vec2 uScale;
uniform mat2 uRotation;
uniform vec2 uTranslation;
uniform float uAspectRatio;

void main(void) {
  vec2 scaled_pos = vec2(pos.x * uScale.x, pos.y * uScale.y);
  vec2 rotated_pos = uRotation * scaled_pos;
  vec2 screen_pos = rotated_pos + uTranslation;

  if (uAspectRatio > 1.0) {
    screen_pos.x /= uAspectRatio;
  }
  else {
    screen_pos.y *= uAspectRatio;
  }

  gl_Position = vec4(screen_pos, 0.0, 1.0);
}

)END"
