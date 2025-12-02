#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

layout (std140) uniform uCommon
{
  mat4 cameraTransform;
  vec4 cameraPosition;
  float time;
};

in VS_OUT {
  vec3 normal;
  vec3 cameraPos;
  vec3 fragPos;

  float explode_time;
  vec4 instance_color;

  float bubble_time;
} gs_in[];

out vec3 normal;
out vec3 cameraPos;
out vec3 fragPos;

out vec4 instance_color;

vec3 GetNormal()
{
   vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[2].gl_Position);
   vec3 b = vec3(gl_in[1].gl_Position) - vec3(gl_in[2].gl_Position);
   return normalize(cross(a, b));
}

vec4 explode(vec4 position, vec3 normal)
{
    float magnitude = max((gs_in[0].bubble_time - gs_in[0].explode_time) * 10.0F, 0);
    vec3 direction = normal * magnitude;
    return position + vec4(direction, 0.0);
}

void main()
{
  vec3 face_normal = GetNormal();

  for(int i = 0; i < 3; i++) {
    normal = gs_in[i].normal;
    cameraPos = gs_in[i].cameraPos;
    instance_color = gs_in[i].instance_color;

    vec4 newPos = explode(gl_in[i].gl_Position, face_normal);
    fragPos = newPos.xyz;
    gl_Position = cameraTransform * newPos;
    EmitVertex();
  }

  EndPrimitive();
}
